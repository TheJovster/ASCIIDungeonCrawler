#define NOMINMAX
#include "ItemDatabase.h"
#include <random>
#include <algorithm>

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    static int randInt(int lo, int hi) {
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    }

    ItemDatabase& ItemDatabase::get() {
        static ItemDatabase instance;
        return instance;
    }

    ItemDatabase::ItemDatabase() {
        // --- materials ---
        // name, bonus, minFloor, valueMultiplier
        m_materials = {
            { "Leather",     1,  1,  1  },
            { "Copper",      2,  1,  2  },
            { "Iron",        3,  2,  3  },
            { "Steel",       5,  3,  5  },
            { "Silver",      7,  4,  8  },
            { "Platinum",    10, 5,  12 },
            { "Black Iron",  13, 6,  16 },
            { "Adamantium",  17, 8,  22 },
        };

        // --- base weapons (slot, baseAttack, baseDefense, baseHP, baseValue) ---
        m_weapons = {
            { "Sword",  EquipSlot::Weapon, 3, 0, 0, 15 },
            { "Dagger", EquipSlot::Weapon, 2, 0, 0, 10 },
        };

        // --- base armors ---
        m_armors = {
            { "Helmet",     EquipSlot::Head,   0, 1, 1, 10 },
            { "Chestplate", EquipSlot::Chest,  0, 2, 2, 20 },
            { "Gauntlets",  EquipSlot::Arms,   0, 1, 1, 10 },
            { "Leggings",   EquipSlot::Legs,   0, 1, 1, 12 },
            { "Boots",      EquipSlot::Boots,  0, 1, 1,  8 },
        };

        // --- base shields ---
        m_shields = {
            { "Shield", EquipSlot::Shield, 0, 2, 0, 15 },
        };

        // --- consumables (static, no material) ---
        m_consumables = {
            { 1, "Small Potion",  ItemType::Consumable, EquipSlot::None, 10, 0, 0, 0, 15 },
            { 2, "Large Potion",  ItemType::Consumable, EquipSlot::None, 25, 0, 0, 0, 30 },
            { 3, "Elixir",        ItemType::Consumable, EquipSlot::None, 50, 0, 0, 0, 60 },
            { 4, "Torch",         ItemType::Equipment, EquipSlot::Torch, 5, 0, 0, 0, 0, 100},
        };

        // --- vendor trash (static) ---
        m_vendorTrash = {
            { 4, "Old Coin",   ItemType::VendorTrash, EquipSlot::None, 5,  0, 0, 0, 0 },
            { 5, "Bone Charm", ItemType::VendorTrash, EquipSlot::None, 8,  0, 0, 0, 0 },
            { 6, "Gem Shard",  ItemType::VendorTrash, EquipSlot::None, 15, 0, 0, 0, 0 },
        };
    }

    Item ItemDatabase::buildItem(const BaseItem& base, const Material& mat) const {
        Item item;
        item.id = m_nextId++;
        item.name = mat.name + " " + base.name;
        item.type = ItemType::Equipment;
        item.slot = base.slot;

        if (base.slot == EquipSlot::Weapon) {
            item.attackBonus = base.baseAttack + mat.bonus;
            item.defenseBonus = 0;
            item.hpBonus = 0;
        }
        else if (base.slot == EquipSlot::Shield) {
            item.attackBonus = 0;
            item.defenseBonus = base.baseDefense + mat.bonus;
            item.hpBonus = 0;
        }
        else {
            // armor slots
            item.attackBonus = 0;
            item.defenseBonus = base.baseDefense + mat.bonus;
            item.hpBonus = base.baseHP + mat.bonus;
        }

        item.value = base.baseValue * mat.valueMultiplier;
        item.healAmount = 0;
        return item;
    }

    Item ItemDatabase::randomEquipment(int floor) const {
        // filter materials available at this floor
        std::vector<const Material*> available;
        for (const auto& mat : m_materials) {
            if (mat.minFloor <= floor)
                available.push_back(&mat);
        }

        // weight towards higher tier materials the deeper the floor
        // higher index = higher tier = more weight per floor
        std::vector<int> weights;
        for (int i = 0; i < (int)available.size(); ++i) {
            // base weight 10, +5 per tier above first, scaled by floor
            int w = 10 + (i * floor * 2);
            weights.push_back(w);
        }

        // weighted random pick
        int total = 0;
        for (int w : weights) total += w;
        int roll = randInt(0, total - 1);
        int cumulative = 0;
        const Material* mat = available.back();
        for (int i = 0; i < (int)available.size(); ++i) {
            cumulative += weights[i];
            if (roll < cumulative) { mat = available[i]; break; }
        }

        // pick random base item type — 40% weapon, 40% armor, 20% shield
        int typeRoll = randInt(0, 9);
        if (typeRoll < 4) {
            const BaseItem& base = m_weapons[randInt(0, (int)m_weapons.size() - 1)];
            // weapons cannot be Leather — bump to Copper
            if (mat->name == "Leather")
                mat = available.size() > 1 ? available[1] : available[0];
            return buildItem(base, *mat);
        }
        else if (typeRoll < 8) {
            const BaseItem& base = m_armors[randInt(0, (int)m_armors.size() - 1)];
            return buildItem(base, *mat);
        }
        else {
            const BaseItem& base = m_shields[randInt(0, (int)m_shields.size() - 1)];
            return buildItem(base, *mat);
        }
    }

    Item ItemDatabase::randomConsumable() const {
        return m_consumables[randInt(0, (int)m_consumables.size() - 1)];
    }

    Item ItemDatabase::randomVendorTrash() const {
        return m_vendorTrash[randInt(0, (int)m_vendorTrash.size() - 1)];
    }

    Item ItemDatabase::randomItem(int floor) const {
        // floor 1: 40% equipment, 40% consumable, 20% vendor trash
        // deeper floors: equipment weight increases, consumable stays, trash decreases
        int equipWeight = 40 + (floor * 3);   // grows with floor
        int consumeWeight = 40;               // stays flat
        int trashWeight = std::max(5, 20 - (floor * 2)); // shrinks with floor, min 5

        int total = equipWeight + consumeWeight + trashWeight;
        int roll = randInt(0, total - 1);

        if (roll < equipWeight)
            return randomEquipment(floor);
        else if (roll < equipWeight + consumeWeight)
            return randomConsumable();
        else
            return randomVendorTrash();
    }

}