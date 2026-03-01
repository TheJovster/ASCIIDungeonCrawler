#include "Renderer.h"
#include "Game.h"
#include "Enemy.h"
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

        // draw map
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
        const Enemy* activeEnemy, int floor) const {
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
        writeStr(row++, "PLAYER");
        writeStr(row++, divider);
        writeStr(row++, "HP:  " + std::to_string(player.hp) + "/" + std::to_string(player.maxHP));
        writeStr(row++, "ATK: " + std::to_string(player.attack));
        writeStr(row++, "DEF: " + std::to_string(player.defense));
        writeStr(row++, "Gold:" + std::to_string(player.gold));
        writeStr(row++, divider);
        writeStr(row++, "FLOOR " + std::to_string(floor));
        writeStr(row++, divider);

        switch (state) {
        case GameState::Exploring:
            writeStr(row++, "EXPLORING");
            writeStr(row++, "");
            writeStr(row++, "");
            writeStr(row++, "");
            writeStr(row++, "");
            writeStr(row++, "[Esc] Quit");
            break;

        case GameState::Combat:
            writeStr(row++, "COMBAT");
            if (activeEnemy) {
                writeStr(row++, "vs " + activeEnemy->getName());
                writeStr(row++, "Enemy HP: " +
                    std::to_string(activeEnemy->getHP()) + "/" +
                    std::to_string(activeEnemy->getMaxHP()));
            }
            else {
                writeStr(row++, "");
                writeStr(row++, "");
            }
            writeStr(row++, divider);
            writeStr(row++, "[Space] Attack");
            writeStr(row++, "[Esc]   Quit");
            break;

        case GameState::GameOver:
            writeStr(row++, "GAME OVER");
            writeStr(row++, "");
            writeStr(row++, "");
            writeStr(row++, divider);
            writeStr(row++, "[Esc] Quit");
            break;
        }
    }
}