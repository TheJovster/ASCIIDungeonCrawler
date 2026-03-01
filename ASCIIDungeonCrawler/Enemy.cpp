#include "Enemy.h"

namespace DungeonGame {

    Enemy::Enemy(int x, int y, EnemyTier tier)
        : Entity(x, y, '?', EntityType::Enemy, ""), m_tier(tier) {
        initFromTier(tier);
    }

    void Enemy::initFromTier(EnemyTier tier) {
        switch (tier) {
        case EnemyTier::Basic:
            m_symbol = '!';
            m_name = "Grunt";
            m_hp = 10;
            m_maxHP = 10;
            m_attack = 3;
            m_defense = 1;
            m_goldDrop = 5;
            break;

        case EnemyTier::Agile:
            m_symbol = '?';
            m_name = "Trickster";
            m_hp = 7;
            m_maxHP = 7;
            m_attack = 5;
            m_defense = 0;
            m_goldDrop = 8;
            break;

        case EnemyTier::Heavy:
            m_symbol = '+';
            m_name = "Brute";
            m_hp = 20;
            m_maxHP = 20;
            m_attack = 4;
            m_defense = 3;
            m_goldDrop = 12;
            break;
        }
    }

}