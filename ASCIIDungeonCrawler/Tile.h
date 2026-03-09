#pragma once

#include "Types.h"

namespace DungeonGame {

    struct Tile {
        TileType type = TileType::Void;
        bool     isExit = false;

        bool visited = false;
        bool visible = false;
    };
}