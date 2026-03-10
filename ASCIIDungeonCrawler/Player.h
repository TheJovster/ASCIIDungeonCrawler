#pragma once
#include "InventorySystem.h"
#include "EquipmentSystem.h"

namespace DungeonGame {

    struct Player {
        int x = 0;
        int y = 0;

        float visualX = 0.f;
        float visualY = 0.f;

        float angle = 0.f;
        float targetAngle = 0.f;

        int baseHP = 30;
        int baseAttack = 5;
        int baseDefense = 2;
        int gold = 0;

        InventorySystem inventory;
        EquipmentSystem equipment;

        int maxHP()   const { return baseHP + equipment.totalHPBonus(); }
        int attack()  const { return baseAttack + equipment.totalAttackBonus(); }
        int defense() const { return baseDefense + equipment.totalDefenseBonus(); }

        int hp = 30; 

        bool isAlive() const { return hp > 0; }
    };

}