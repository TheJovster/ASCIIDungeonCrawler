#pragma once
#include "Item.h"
#include <vector>
#include <string>

namespace DungeonGame {

    struct Material {
        std::string name;
        int         bonus;
        int         minFloor;
        int         valueMultiplier;
    };

    struct BaseItem {
        std::string name;
        EquipSlot   slot;
        int         baseAttack;
        int         baseDefense;
        int         baseHP;
        int         baseValue;
    };

    class ItemDatabase {
    public:
        static ItemDatabase& get();

        Item randomItem(int floor = 1) const;
        Item randomEquipment(int floor) const;
        Item randomConsumable() const;
        Item randomVendorTrash() const;

    private:
        ItemDatabase();

        Item buildItem(const BaseItem& base, const Material& mat) const;

        std::vector<Material> m_materials;
        std::vector<BaseItem> m_weapons;
        std::vector<BaseItem> m_armors;
        std::vector<BaseItem> m_shields;
        std::vector<Item>     m_consumables;
        std::vector<Item>     m_vendorTrash;

        mutable int m_nextId = 100; // dynamic items start at 100
    };

}