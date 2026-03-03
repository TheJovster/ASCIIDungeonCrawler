#pragma once

namespace DungeonGame {

    enum class Action {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        Interact,
        ToggleInventory,
        Confirm,    
        Deny,       
        Quit,
        None
    };

    Action getInput();

}