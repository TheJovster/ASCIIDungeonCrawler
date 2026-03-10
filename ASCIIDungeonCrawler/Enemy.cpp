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
            // animation
            m_animator.addClip(AnimationState::IdlePassive, {
                {
                    "assets/animations/enemy_trickster_idle_1.png",
                    "assets/animations/enemy_trickster_idle_2.png",
                    "assets/animations/enemy_trickster_idle_3.png",
                    "assets/animations/enemy_trickster_idle_4.png",
                    "assets/animations/enemy_trickster_idle_5.png",
                    "assets/animations/enemy_trickster_idle_6.png",
                    "assets/animations/enemy_trickster_idle_7.png",
                    "assets/animations/enemy_trickster_idle_8.png",
                    "assets/animations/enemy_trickster_idle_9.png",
                    "assets/animations/enemy_trickster_idle_10.png",
                    "assets/animations/enemy_trickster_idle_11.png",
                    "assets/animations/enemy_trickster_idle_12.png"
                }, true, 12.f });
            m_animator.addClip(AnimationState::Attack, {
                {
                    "assets/animations/enemy_trickster_attack_1.png",
                    "assets/animations/enemy_trickster_attack_2.png",
                    "assets/animations/enemy_trickster_attack_3.png",
                    "assets/animations/enemy_trickster_attack_4.png",
                    "assets/animations/enemy_trickster_attack_5.png",
                    "assets/animations/enemy_trickster_attack_6.png",
                    "assets/animations/enemy_trickster_attack_7.png",
                    "assets/animations/enemy_trickster_attack_8.png",
                    "assets/animations/enemy_trickster_attack_9.png",
                    "assets/animations/enemy_trickster_attack_10.png",
                    "assets/animations/enemy_trickster_attack_11.png",
                    "assets/animations/enemy_trickster_attack_12.png"
                }, false, 12.f });
            m_animator.addClip(AnimationState::Hit, {
                {
                    "assets/animations/enemy_trickster_hit_1.png",
                    "assets/animations/enemy_trickster_hit_2.png",
                    "assets/animations/enemy_trickster_hit_3.png",
                    "assets/animations/enemy_trickster_hit_4.png",
                    "assets/animations/enemy_trickster_hit_5.png",
                    "assets/animations/enemy_trickster_hit_6.png",
                    "assets/animations/enemy_trickster_hit_7.png",
                    "assets/animations/enemy_trickster_hit_8.png",
                    "assets/animations/enemy_trickster_hit_9.png",
                    "assets/animations/enemy_trickster_hit_10.png",
                    "assets/animations/enemy_trickster_hit_11.png",
                    "assets/animations/enemy_trickster_hit_12.png"
                }, false, 12.f });
            m_animator.setState(AnimationState::IdlePassive);
            m_animator.setReturnState(AnimationState::IdlePassive);
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

    void Enemy::scaleToFloor(int floor) {
        if (floor <= 1) return;
        int depth = floor - 1;

        m_hp += depth * 4;
        m_maxHP += depth * 4;
        m_attack += depth * 1;
        m_defense += depth / 2; 
        m_goldDrop += depth * 2;
    }

}