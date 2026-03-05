#pragma once
#include <string>

namespace DungeonGame {

    enum class ItemType {
        Equipment,
        Consumable,
        VendorTrash
    };

    enum class EquipSlot {
        None,
        Head,
        Chest,
        Arms,
        Legs,
        Boots,
        Weapon,
        Shield, 
        Torch
    };

    struct Item {
        int         id = 0;
        std::string name = "";
        ItemType    type = ItemType::VendorTrash;
        EquipSlot   slot = EquipSlot::None;
        int         value = 0;

        int         hpBonus = 0;
        int         attackBonus = 0;
        int         defenseBonus = 0;

        int         healAmount = 0;

        int         charges = 0;
    };

}