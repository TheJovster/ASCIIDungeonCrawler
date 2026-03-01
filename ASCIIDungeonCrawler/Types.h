#pragma once

#pragma once

namespace DungeonGame {

    constexpr int MAP_WIDTH = 80;
    constexpr int MAP_HEIGHT = 30;

    constexpr int MIN_ROOMS = 6;
    constexpr int MAX_ROOMS = 12;

    constexpr int MIN_ROOM_SIZE = 4;
    constexpr int MAX_ROOM_SIZE = 10;

    enum class TileType {
        Void,
        Floor,
        WallH,   
        WallV   
    };

}