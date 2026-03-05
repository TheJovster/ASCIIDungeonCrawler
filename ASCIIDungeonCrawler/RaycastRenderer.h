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
        float width; // relative size multiplier
    };

    public:
        RaycastRenderer();
        void draw(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player);
        void drawMinimap(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player);

    private:
        static constexpr float PI = 3.14159265f;
        static constexpr float FOV = PI / 3.f;      
        static constexpr float HALF_FOV = FOV / 2.f;
        static constexpr int   MAP_DRAW_WIDTH = 800;
        static constexpr int   MINIMAP_SCALE = 4;   
        static constexpr int   MINIMAP_X = 820;     
        static constexpr int   MINIMAP_Y = 10;   

        sf::VertexArray m_lines; // reused each frame
        std::array<float, 800> m_zBuffer;

        bool isWall(const Dungeon& dungeon, int tx, int ty) const;
        sf::Color wallColor(float distance, float brightness) const;
        void drawSprites(sf::RenderWindow& window, const Dungeon& dungeon, const Player& player);
    };

}