#include "EquipmentSystem.h"

namespace DungeonGame {

    std::optional<Item> EquipmentSystem::equip(const Item& item) {
        std::optional<Item>* slot = nullptr;
        switch (item.slot) {
        case EquipSlot::Head:   slot = &head;   break;
        case EquipSlot::Chest:  slot = &chest;  break;
        case EquipSlot::Arms:   slot = &arms;   break;
        case EquipSlot::Legs:   slot = &legs;   break;
        case EquipSlot::Boots:  slot = &boots;  break;
        case EquipSlot::Weapon: slot = &weapon; break;
        case EquipSlot::Shield: slot = &shield; break;
        default: return std::nullopt;
        }
        std::optional<Item> previous = *slot;
        *slot = item;
        return previous; 
    }

    int EquipmentSystem::totalAttackBonus() const {
        int total = 0;
        if (weapon) total += weapon->attackBonus;
        return total;
    }

    int EquipmentSystem::totalDefenseBonus() const {
        int total = 0;
        if (head)   total += head->defenseBonus;
        if (chest)  total += chest->defenseBonus;
        if (arms)   total += arms->defenseBonus;
        if (legs)   total += legs->defenseBonus;
        if (boots)  total += boots->defenseBonus;
        if (shield) total += shield->defenseBonus;
        return total;
    }

    int EquipmentSystem::totalHPBonus() const {
        int total = 0;
        if (head)   total += head->hpBonus;
        if (chest)  total += chest->hpBonus;
        if (arms)   total += arms->hpBonus;
        if (legs)   total += legs->hpBonus;
        if (boots)  total += boots->hpBonus;
        return total;
    }

    const std::optional<Item>& EquipmentSystem::getSlot(EquipSlot slot) const {
        switch (slot) {
        case EquipSlot::Head:   return head;
        case EquipSlot::Chest:  return chest;
        case EquipSlot::Arms:   return arms;
        case EquipSlot::Legs:   return legs;
        case EquipSlot::Boots:  return boots;
        case EquipSlot::Weapon: return weapon;
        case EquipSlot::Shield: return shield;
        default:                return head; // unreachable
        }
    }

    std::string EquipmentSystem::slotName(EquipSlot slot) const {
        switch (slot) {
        case EquipSlot::Head:   return "Head  ";
        case EquipSlot::Chest:  return "Chest ";
        case EquipSlot::Arms:   return "Arms  ";
        case EquipSlot::Legs:   return "Legs  ";
        case EquipSlot::Boots:  return "Boots ";
        case EquipSlot::Weapon: return "Weapon";
        case EquipSlot::Shield: return "Shield";
        default:                return "      ";
        }
    }

}