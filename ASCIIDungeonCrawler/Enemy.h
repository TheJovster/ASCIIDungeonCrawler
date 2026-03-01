#pragma once
#include "Entity.h"

namespace DungeonGame {

    enum class EnemyTier {
        Basic,   
        Agile,   
        Heavy    
    };

    class Enemy : public Entity {
    public:
        Enemy(int x, int y, EnemyTier tier);

        int getHP()      const { return m_hp; }
        int getMaxHP()   const { return m_maxHP; }
        int getAttack()  const { return m_attack; }
        int getDefense() const { return m_defense; }
        int getGoldDrop() const { return m_goldDrop; }
        EnemyTier getTier() const { return m_tier; }

        void takeDamage(int amount) { m_hp -= amount; if (m_hp <= 0) kill(); }

    private:
        EnemyTier m_tier;
        int m_hp = 0;
        int m_maxHP = 0;
        int m_attack = 0;
        int m_defense = 0;
        int m_goldDrop = 0;

        void initFromTier(EnemyTier tier);
    };

}