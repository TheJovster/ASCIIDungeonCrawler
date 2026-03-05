#pragma once
#include "Item.h"
#include <optional>

namespace DungeonGame {

    struct EquipmentSystem {
        std::optional<Item> head;
        std::optional<Item> chest;
        std::optional<Item> arms;
        std::optional<Item> legs;
        std::optional<Item> boots;
        std::optional<Item> weapon;
        std::optional<Item> shield;
        std::optional<Item> torch;

        std::optional<Item> equip(const Item& item);

        int totalAttackBonus()  const;
        int totalDefenseBonus() const;
        int totalHPBonus()      const;

        const std::optional<Item>& getSlot(EquipSlot slot) const;
        std::string slotName(EquipSlot slot) const;
    };

}