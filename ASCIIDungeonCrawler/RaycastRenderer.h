#pragma once
#include "Dungeon.h"
#include "Player.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <array>
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
        RaycastRenderer();
        void draw(sf::RenderWindow& window, const Dungeon& dungeon,
            const Player& player, float dt);
        void drawMinimap(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player);

        //effects
        void triggerHitFlash();
        void triggerCritFlash();
        void updateOverlays(sf::RenderWindow& ,float dt);

    private:
        static constexpr float PI = 3.14159265f;
        static constexpr float FOV = PI / 3.f;
        static constexpr float HALF_FOV = FOV / 2.f;
        static constexpr int   MAP_DRAW_WIDTH = 800;
        static constexpr int   MINIMAP_SCALE = 4;
        static constexpr int   MINIMAP_X = SCREEN_WIDTH - (MAP_WIDTH * MINIMAP_SCALE) - 10;
        static constexpr int   MINIMAP_Y = SCREEN_HEIGHT - (MAP_HEIGHT * MINIMAP_SCALE) - 10;
        static constexpr int   TEX_SIZE = 1024;

        //effects
        float m_hitVignetteTimer = 0.f;
        float m_critFlashTimer = 0.f;
        static constexpr float HIT_VIGNETTE_DURATION = 0.5f;
        static constexpr float CRIT_FLASH_DURATION = 0.15f;

        // wall texture
        sf::Image   m_wallImage;
        sf::Texture m_wallTexture;

        // floor/ceiling pixel buffer
        sf::Image   m_floorBuffer;
        sf::Texture m_floorTexture;
        sf::Sprite  m_floorSprite;
        sf::Image   m_floorImage;
        sf::Image   m_ceilImage;

        // entities images
        std::unordered_map<std::string, sf::Texture> m_spriteTextures;

        sf::VertexArray        m_lines;
        std::array<float, 800> m_zBuffer{};
        float                  m_time = 0.f;

        bool      isWall(const Dungeon& dungeon, int tx, int ty) const;
        sf::Color wallColor(float distance, float brightness, float lightRadius) const;
        void      drawSprites(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player, float lightRadius);
        void      drawFloorCeiling(sf::RenderWindow& window, const Player& player, float lightRadius);

        const sf::Texture* getSpriteTexture(const std::string& path);
    };

}