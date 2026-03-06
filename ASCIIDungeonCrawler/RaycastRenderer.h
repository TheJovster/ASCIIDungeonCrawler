#pragma once
#include "Dungeon.h"
#include "Player.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <array>

namespace DungeonGame {

    class RaycastRenderer {

    private:
        struct Sprite {
            float worldX;
            float worldY;
            sf::Color color;
            float width;

            const sf::Image* image = nullptr;
            float verticalOffset = 0.f;
        };

    public:
        RaycastRenderer();
        void draw(sf::RenderWindow& window, const Dungeon& dungeon,
            const Player& player, float dt);
        void drawMinimap(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player);

    private:
        static constexpr float PI = 3.14159265f;
        static constexpr float FOV = PI / 3.f;
        static constexpr float HALF_FOV = FOV / 2.f;
        static constexpr int   MAP_DRAW_WIDTH = 800;
        static constexpr int   MINIMAP_SCALE = 4;
        static constexpr int   MINIMAP_X = SCREEN_WIDTH - (MAP_WIDTH * MINIMAP_SCALE) - 10;
        static constexpr int   MINIMAP_Y = SCREEN_HEIGHT - (MAP_HEIGHT * MINIMAP_SCALE) - 10;
        static constexpr int   TEX_SIZE = 1024;

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
        //sf::Image m_enemyImage; //commented out for now - wait - I will need different images for different enemies. I'll figure it out later
        sf::Image m_merchantImage;
        sf::Image m_chestClosedImage;
        sf::Image m_chestOpenedImage;
        sf::Image m_exitImage;

        sf::VertexArray        m_lines;
        std::array<float, 800> m_zBuffer{};
        float                  m_time = 0.f;

        bool      isWall(const Dungeon& dungeon, int tx, int ty) const;
        sf::Color wallColor(float distance, float brightness, float lightRadius) const;
        void      drawSprites(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player, float lightRadius);
        void      drawFloorCeiling(sf::RenderWindow& window, const Player& player, float lightRadius);
    };

}