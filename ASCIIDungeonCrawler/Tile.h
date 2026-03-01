#pragma once

#include "Types.h"

namespace DungeonGame {

    struct Tile {
        TileType type = TileType::Void;
        bool     hasChest = false;
        bool     isExit = false;
        
        //TODO - a vector of entities in this room - if necessary
    };

}

