#pragma once

#pragma once

namespace DungeonGame {

    constexpr int MIN_ROOMS = 6;
    constexpr int MAX_ROOMS = 12;

    constexpr int MIN_ROOM_SIZE = 4;
    constexpr int MAX_ROOM_SIZE = 10;

    constexpr int MAP_WIDTH = 80;
    constexpr int MAP_HEIGHT = 30;
    constexpr int CONSOLE_WIDTH = 120;
    constexpr int HUD_COL = 90;  // start column of HUD (80 + 10 padding)
    constexpr int HUD_WIDTH = 30;

    enum class TileType {
        Void,
        Floor,
        WallH,   
        WallV   
    };

}