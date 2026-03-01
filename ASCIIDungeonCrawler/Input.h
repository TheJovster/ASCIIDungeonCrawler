#pragma once

namespace DungeonGame {

    enum class Action {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        Interact,
        Quit,
        None
    };

    Action getInput();

}