#include "Renderer.h"
#include "Game.h"
#include "Enemy.h"
#include "InventorySystem.h"
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <unordered_map>

namespace DungeonGame {

    static char tileToChar(const Tile& tile) {
        switch (tile.type) {
        case TileType::Floor:  return '.';
        case TileType::WallH:  return '-';
        case TileType::WallV:  return '|';
        case TileType::Void:
        default:               return '#';
        }
    }

    void Renderer::drawMap(const Dungeon& dungeon, const Player& player,
        const std::vector<std::string>& log,
        GameState state) const {

        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        const auto& grid = dungeon.getGrid();
        const auto& entities = dungeon.getEntities();

        std::unordered_map<int, char> entitySymbols;
        for (const auto& e : entities) {
            if (e->isAlive()) {
                int key = e->getY() * MAP_WIDTH + e->getX();
                entitySymbols[key] = e->getSymbol();
            }
        }

        auto writeChar = [&](int col, int row, char c) {
            COORD pos = { (SHORT)col, (SHORT)row };
            DWORD written;
            WriteConsoleOutputCharacterA(console, &c, 1, pos, &written);
            };

        for (int row = 0; row < MAP_HEIGHT; ++row) {
            for (int col = 0; col < MAP_WIDTH; ++col) {
                char c;
                if (row == player.y && col == player.x) {
                    c = '@';
                }
                else {
                    int key = row * MAP_WIDTH + col;
                    auto it = entitySymbols.find(key);
                    if (it != entitySymbols.end()) {
                        c = it->second;
                    }
                    else {
                        const Tile& tile = grid[row][col];
                        if (tile.isExit)   c = '>';
                        else if (tile.hasChest) c = 'C';
                        else c = tileToChar(tile);
                    }
                }
                writeChar(col, row, c);
            }
        }

        for (int col = 0; col < MAP_WIDTH; ++col)
            writeChar(col, MAP_HEIGHT, '-');


        int logSize = (int)log.size();
        int start = std::max(0, logSize - 3);
        for (int i = 0; i < 3; ++i) {
            std::string line = (start + i < logSize) ? log[start + i] : "";

            line.resize(MAP_WIDTH, ' ');
            for (int col = 0; col < MAP_WIDTH; ++col)
                writeChar(col, MAP_HEIGHT + 1 + i, line[col]);
        }
    }

    void Renderer::drawHUD(const Player& player, GameState state,
        const Enemy* activeEnemy, int floor, bool inventoryMode) const {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        auto writeStr = [&](int row, const std::string& text) {
            std::string line = text;
            if ((int)line.size() < HUD_WIDTH)
                line += std::string(HUD_WIDTH - line.size(), ' ');
            line = line.substr(0, HUD_WIDTH);
            COORD pos = { (SHORT)HUD_COL, (SHORT)row };
            DWORD written;
            WriteConsoleOutputCharacterA(console, line.c_str(), (DWORD)line.size(), pos, &written);
            };

        std::string divider(HUD_WIDTH, '-');
        int row = 0;

        // --- player stats ---
        writeStr(row++, "PLAYER");
        writeStr(row++, divider);
        writeStr(row++, "HP:  " + std::to_string(player.hp) + "/" + std::to_string(player.maxHP()));
        writeStr(row++, "ATK: " + std::to_string(player.attack()) +
            "  DEF: " + std::to_string(player.defense()));
        writeStr(row++, "Gold: " + std::to_string(player.gold));
        writeStr(row++, divider);

        // --- equipment slots ---
        auto slotStr = [&](const std::string& label, const std::optional<Item>& slot) {
            std::string val = slot.has_value() ? slot->name : "none";
            std::string line = "[" + label + "] " + val;
            return line;
            };

        writeStr(row++, slotStr("Head  ", player.equipment.head));
        writeStr(row++, slotStr("Chest ", player.equipment.chest));
        writeStr(row++, slotStr("Arms  ", player.equipment.arms));
        writeStr(row++, slotStr("Legs  ", player.equipment.legs));
        writeStr(row++, slotStr("Boots ", player.equipment.boots));
        writeStr(row++, slotStr("Weapon", player.equipment.weapon));
        writeStr(row++, slotStr("Shield", player.equipment.shield));
        writeStr(row++, divider);

        // --- inventory ---
        const auto& inv = player.inventory;
        int         invCount = inv.count();
        int         invCap = inv.capacity();
        writeStr(row++, "INV: " + std::to_string(invCount) + "/" + std::to_string(invCap));

        // visible window into inventory list
        int selectedIndex = inv.getSelectedIndex();
        int scrollOffset = inv.getScrollOffset();
        bool scrollable = invCount >= InventorySystem::SCROLL_THRESHOLD;

        for (int i = 0; i < InventorySystem::VISIBLE_ROWS; ++i) {
            int itemIndex = scrollOffset + i;
            if (itemIndex >= invCount) {
                writeStr(row++, "");
                continue;
            }
            const Item& item = inv.getItem(itemIndex);
            bool        selected = scrollable && (itemIndex == selectedIndex);

            // truncate name to leave room for selector
            std::string name = item.name;
            int         maxNameLen = HUD_WIDTH - 5; // room for " <--"
            if ((int)name.size() > maxNameLen)
                name = name.substr(0, maxNameLen);

            std::string line = name;
            if (selected) {
                // pad name then append selector
                while ((int)line.size() < maxNameLen)
                    line += ' ';
                line += " <--";
            }
            writeStr(row++, line);
        }

        writeStr(row++, divider);

        // --- floor + game state ---
        writeStr(row++, "FLOOR " + std::to_string(floor));
        writeStr(row++, divider);

        switch (state) {
        case GameState::Exploring:
            writeStr(row++, inventoryMode ? "MODE: INVENTORY" : "MODE: EXPLORE");
            writeStr(row++, "[Tab]  Switch Mode");
            writeStr(row++, "[Esc]  Menu");
            break;

        case GameState::Combat:
            writeStr(row++, "COMBAT");
            if (activeEnemy) {
                writeStr(row++, "vs " + activeEnemy->getName());
                writeStr(row++, "HP: " +
                    std::to_string(activeEnemy->getHP()) + "/" +
                    std::to_string(activeEnemy->getMaxHP()));
            }
            else {
                writeStr(row++, "");
                writeStr(row++, "");
            }
            writeStr(row++, divider);
            writeStr(row++, "[Space] Attack");
            writeStr(row++, "[Esc]   Menu");
            break;

        case GameState::GameOver:
            writeStr(row++, "GAME OVER");
            writeStr(row++, "");
            writeStr(row++, "[Esc] Menu");
            break;
        }
    }
}