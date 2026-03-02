#pragma once

namespace DungeonGame {

    enum class Action {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        Interact,
        ToggleInventory,
        Quit,
        None
    };

    Action getInput();

}