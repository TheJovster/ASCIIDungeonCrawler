#pragma once
#include "Dungeon.h"
#include "Player.h"
#include "Merchant.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace DungeonGame {

    enum class GameState;

    class Renderer {
    public:
        Renderer();

        void drawMap(sf::RenderWindow& window, const Dungeon& dungeon,
            const Player& player,
            const std::vector<std::string>& log,
            GameState state) const;

        void drawHUD(sf::RenderWindow& window,
            const Player& player, GameState state,
            const Enemy* activeEnemy, int floor,
            bool inventoryMode,
            const std::vector<Item>* chestContents,
            int chestSelected,
            int inventoryActionSelected,
            const Merchant* activeMerchant,
            MerchantMode merchantMode,
            int merchantTopSelected,
            int sellIndex) const;

    private:
        sf::Font m_font;

        static constexpr int CHAR_SIZE = 14;
        static constexpr int CELL_W = 10;
        static constexpr int CELL_H = 16;
        static constexpr int HUD_PIXEL_X = 820; // HUD starts here in pixels
    };

}