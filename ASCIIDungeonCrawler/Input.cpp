#include "Input.h"

namespace DungeonGame {

    Action getInput(const sf::Event& event) {
        if (event.type != sf::Event::KeyPressed)
            return Action::None;

        switch (event.key.code) {
        case sf::Keyboard::W:      return Action::MoveUp;
        case sf::Keyboard::S:      return Action::MoveDown;
        case sf::Keyboard::A:      return Action::MoveLeft;
        case sf::Keyboard::D:      return Action::MoveRight;
        case sf::Keyboard::Q:      return Action::RotateLeft;
        case sf::Keyboard::E:      return Action::RotateRight;
        case sf::Keyboard::Space:  return Action::Interact;
        case sf::Keyboard::Tab:    return Action::ToggleInventory;
        case sf::Keyboard::Escape: return Action::Quit;
        case sf::Keyboard::Y:      return Action::Confirm;
        case sf::Keyboard::N:      return Action::Deny;
        default:                   return Action::None;
        }
    }

}