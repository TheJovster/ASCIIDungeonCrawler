#pragma once
#include <SFML/Graphics.hpp>

namespace DungeonGame {

    enum class Action {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        RotateLeft,
        RotateRight,
        Interact,
        ToggleInventory,
        Confirm,
        Deny,
        Quit,
        None
    };

    Action getInput(const sf::Event& event);

}