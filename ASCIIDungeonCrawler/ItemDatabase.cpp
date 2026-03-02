#include "ItemDatabase.h"
#include <random>
#include <stdexcept>

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    template<typename T>
    static const T& randomFrom(const std::vector<T>& vec) {
        return vec[std::uniform_int_distribution<int>(0, (int)vec.size() - 1)(rng)];
    }

    const ItemDatabase& ItemDatabase::get() {
        static ItemDatabase instance;
        return instance;
    }

    ItemDatabase::ItemDatabase() {

        m_consumables.push_back({ 1, "Small Potion",  ItemType::Consumable, EquipSlot::None, 10,  0, 0, 0, 15 });
        m_consumables.push_back({ 2, "Large Potion",  ItemType::Consumable, EquipSlot::None, 25,  0, 0, 0, 35 });
        m_consumables.push_back({ 3, "Elixir",        ItemType::Consumable, EquipSlot::None, 50,  0, 0, 0, 60 });


        m_equipment.push_back({ 10, "Leather Cap",    ItemType::Equipment, EquipSlot::Head,   20,  5, 0, 1, 0 });
        m_equipment.push_back({ 11, "Iron Helmet",    ItemType::Equipment, EquipSlot::Head,   50, 10, 0, 2, 0 });
        m_equipment.push_back({ 12, "Leather Vest",   ItemType::Equipment, EquipSlot::Chest,  30,  5, 0, 2, 0 });
        m_equipment.push_back({ 13, "Chain Mail",     ItemType::Equipment, EquipSlot::Chest,  80, 15, 0, 4, 0 });
        m_equipment.push_back({ 14, "Leather Gloves", ItemType::Equipment, EquipSlot::Arms,   15,  0, 0, 1, 0 });
        m_equipment.push_back({ 15, "Iron Gauntlets", ItemType::Equipment, EquipSlot::Arms,   40,  0, 0, 2, 0 });
        m_equipment.push_back({ 16, "Leather Boots",  ItemType::Equipment, EquipSlot::Boots,  15,  0, 0, 1, 0 });
        m_equipment.push_back({ 17, "Iron Boots",     ItemType::Equipment, EquipSlot::Boots,  40,  0, 0, 2, 0 });
        m_equipment.push_back({ 18, "Leather Pants",  ItemType::Equipment, EquipSlot::Legs,   20,  5, 0, 1, 0 });
        m_equipment.push_back({ 19, "Iron Leggings",  ItemType::Equipment, EquipSlot::Legs,   55, 10, 0, 3, 0 });
        m_equipment.push_back({ 20, "Dagger",         ItemType::Equipment, EquipSlot::Weapon, 30,  0, 2, 0, 0 });
        m_equipment.push_back({ 21, "Short Sword",    ItemType::Equipment, EquipSlot::Weapon, 60,  0, 4, 0, 0 });
        m_equipment.push_back({ 22, "Broad Sword",    ItemType::Equipment, EquipSlot::Weapon,120,  0, 7, 0, 0 });
        m_equipment.push_back({ 23, "Wooden Shield",  ItemType::Equipment, EquipSlot::Shield, 25,  0, 0, 2, 0 });
        m_equipment.push_back({ 24, "Iron Shield",    ItemType::Equipment, EquipSlot::Shield, 70,  0, 0, 4, 0 });

        m_vendorTrash.push_back({ 30, "Old Coin",     ItemType::VendorTrash, EquipSlot::None, 5,  0, 0, 0, 0 });
        m_vendorTrash.push_back({ 31, "Gem Fragment", ItemType::VendorTrash, EquipSlot::None, 15, 0, 0, 0, 0 });
        m_vendorTrash.push_back({ 32, "Bone Charm",   ItemType::VendorTrash, EquipSlot::None, 10, 0, 0, 0, 0 });
    }

    Item ItemDatabase::randomConsumable()  const { return randomFrom(m_consumables); }
    Item ItemDatabase::randomEquipment()   const { return randomFrom(m_equipment); }
    Item ItemDatabase::randomVendorTrash() const { return randomFrom(m_vendorTrash); }

    Item ItemDatabase::randomItem() const {
        
        int roll = std::uniform_int_distribution<int>(1, 10)(rng);
        if (roll <= 4)  return randomEquipment();
        if (roll <= 8)  return randomConsumable();
        return randomVendorTrash();
    }

}