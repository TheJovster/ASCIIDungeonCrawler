#pragma once
#include "Dungeon.h"
#include "Player.h"
#include "Merchant.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "CombatSystem.h"
#include "InventorySystem.h"

namespace DungeonGame {

    enum class GameState;

    struct CombatHUDData {
        CombatPhase phase;
        int actionSelected;
        int itemSelected;
        const std::vector<int>* itemList;
        const InventorySystem* playerInventory;  // changed
        const std::string* lastAction;
        const std::vector<std::string>* history;
    };

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
            int sellIndex,
            const CombatHUDData& combatData) const;

    private:
        sf::Font m_font;

        static constexpr int CHAR_SIZE = 14;
        static constexpr int CELL_W = 10;
        static constexpr int CELL_H = 16;
        static constexpr int HUD_PIXEL_X = 820; // HUD starts here in pixels
    };

}