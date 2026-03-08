#pragma once
#include "Dungeon.h"
#include "Player.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>

namespace DungeonGame {

    class RaycastRenderer {
    private:
        struct Sprite {
            float worldX;
            float worldY;
            sf::Color color;
            float width;
            const sf::Texture* texture = nullptr;
            float verticalOffset = 0.f;
        };

    public:
        RaycastRenderer(sf::RenderWindow& window);
        void draw(sf::RenderWindow& window, const Dungeon& dungeon,
            const Player& player, float dt);
        void drawMinimap(sf::RenderWindow& window, const Dungeon& dungeon,
            const Player& player);

        void triggerHitFlash();
        void triggerCritFlash();
        void updateOverlays(sf::RenderWindow& window, float dt);

    private:
        // runtime dimensions — derived from window at construction
        int   m_screenW = 0;
        int   m_screenH = 0;
        int   m_drawWidth = 0; // raycaster panel width (62% of screen)
        int   m_minimapX = 0;
        int   m_minimapY = 0;

        static constexpr int   MINIMAP_SCALE = 4;
        static constexpr int   TEX_SIZE = 1024;
        static constexpr float PI = 3.14159265f;

        // effects
        float m_hitVignetteTimer = 0.f;
        float m_critFlashTimer = 0.f;
        static constexpr float HIT_VIGNETTE_DURATION = 0.5f;
        static constexpr float CRIT_FLASH_DURATION = 0.15f;

        // textures
        sf::Image   m_wallImage;
        sf::Image   m_floorBuffer;
        sf::Texture m_floorTexture;
        sf::Sprite  m_floorSprite;
        sf::Image   m_floorImage;
        sf::Image   m_ceilImage;

        std::unordered_map<std::string, sf::Texture> m_spriteTextures;
        std::vector<float> m_zBuffer;

        sf::VertexArray m_lines;
        float           m_time = 0.f;

        bool      isWall(const Dungeon& dungeon, int tx, int ty) const;
        sf::Color wallColor(float distance, float brightness, float lightRadius) const;
        void      drawSprites(sf::RenderWindow& window, const Dungeon& dungeon,
            const Player& player, float lightRadius);
        void      drawFloorCeiling(sf::RenderWindow& window, const Player& player,
            float lightRadius);
        const sf::Texture* getSpriteTexture(const std::string& path);
    };
}