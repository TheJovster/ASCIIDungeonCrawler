#pragma once
#include "Entity.h"

namespace DungeonGame {

    enum class EnemyType {
        Basic,   
        Agile,   
        Heavy    
    };

    class Enemy : public Entity {
    public:
        Enemy(int x, int y, EnemyType type);

        int getHP()      const { return m_hp; }
        int getMaxHP()   const { return m_maxHP; }
        int getAttack()  const { return m_attack; }
        int getDefense() const { return m_defense; }
        int getGoldDrop() const { return m_goldDrop; }
        EnemyType getType() const { return m_type; }

        void takeDamage(int amount) { m_hp -= amount; if (m_hp <= 0) kill(); }
        void scaleToFloor(int floor);
    private:
        EnemyType m_type;
        int m_hp = 0;
        int m_maxHP = 0;
        int m_attack = 0;
        int m_defense = 0;
        int m_goldDrop = 0;
        void initFromTier(EnemyType tier);

        std::string getTextureName() const override {
            const std::string& frame = m_animator.getCurrentFrame();
            if (!frame.empty()) return frame;
            // fallback — in case there are no animations
            switch (m_type) {
            case EnemyType::Basic:  return "assets/enemy_grunt.png";
            case EnemyType::Agile:  return "assets/enemy_trickster.png";
            case EnemyType::Heavy:  return "assets/enemy_brute.png";
            default:                return "";
            }
        }

        void initAnimsTrickster();
        void initAnimsBrute();
        void initAnimsGrunt();
    };

}