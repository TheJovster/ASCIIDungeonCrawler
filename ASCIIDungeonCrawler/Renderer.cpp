#include "Renderer.h"
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

    void Renderer::drawMap(const Dungeon& dungeon, const Player& player) const {
        COORD topLeft = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), topLeft);

        const auto& grid = dungeon.getGrid();
        const auto& entities = dungeon.getEntities();

        std::unordered_map<int, char> entitySymbols;
        for (const auto& e : entities) {
            if (e->isAlive()) {
                int key = e->getY() * MAP_WIDTH + e->getX();
                entitySymbols[key] = e->getSymbol();
            }
        }

        std::string frame;
        frame.reserve(MAP_WIDTH * MAP_HEIGHT + MAP_HEIGHT);

        for (int row = 0; row < MAP_HEIGHT; ++row) {
            for (int col = 0; col < MAP_WIDTH; ++col) {
                if (row == player.y && col == player.x) {
                    frame += '@';
                    continue;
                }

                int key = row * MAP_WIDTH + col;
                auto it = entitySymbols.find(key);
                if (it != entitySymbols.end()) {
                    frame += it->second;
                    continue;
                }

                const Tile& tile = grid[row][col];
                if (tile.isExit) { frame += '>'; continue; }
                if (tile.hasChest) { frame += 'C'; continue; }

                frame += tileToChar(tile);
            }
            if (row < MAP_HEIGHT - 1)
                frame += '\n';
        }

        std::cout << frame << std::flush;
    }
}