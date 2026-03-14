#include "Enemy.h"

namespace DungeonGame {

    Enemy::Enemy(int x, int y, EnemyTier tier)
        : Entity(x, y, '?', EntityType::Enemy, ""), m_tier(tier) {
        initFromTier(tier);
    }

    // Generates frames. The path to the asset is usually assets/animations/animation_name_i.png - i is the number of frames
    // The path is updated when the assets are moved around in the assets folder.
    static std::vector<std::string> makeFrames(const std::string& path, int numberOfFrames) {
        std::vector<std::string> frames;
        for (int i = 1; i <= numberOfFrames; ++i)
            frames.push_back(path + std::to_string(i) + ".png");
        return frames;
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
            initAnimsTrickster();
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

    void Enemy::initAnimsTrickster()
    {
        m_animator.addClip(AnimationState::IdlePassive, {{makeFrames("assets/animations/trickster/enemy_trickster_idle_", 12)}, true, 12.f });
        m_animator.addClip(AnimationState::Attack, {{makeFrames("assets/animations/trickster/enemy_trickster_attack_", 12)}, false, 12.f });
        m_animator.addClip(AnimationState::Hit,{{makeFrames("assets/animations/trickster/enemy_trickster_hit_", 12)}, false, 12.f });
        m_animator.addClip(AnimationState::Die, { makeFrames("assets/animations/trickster/enemy_trickster_death_", 12), false, 12.f });
        m_animator.addClip(AnimationState::Dead, { { "assets/animations/trickster/enemy_trickster_dead.png" }, true, 1.f });
        m_animator.setState(AnimationState::IdlePassive);
        m_animator.setReturnState(AnimationState::IdlePassive);
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