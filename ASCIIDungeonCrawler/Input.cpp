#include "Input.h"

namespace DungeonGame {

    Action getInput(const sf::Event& event) {
        // keyboard
        if (event.type == sf::Event::KeyPressed) {
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

        // controller — only if connected
        if (sf::Joystick::isConnected(0)) {
            if (event.type == sf::Event::JoystickButtonPressed
                && event.joystickButton.joystickId == 0) {
                switch (event.joystickButton.button) {
                case 0: return Action::Interact;
                case 1: return Action::Quit;
                case 2: return Action::ToggleInventory;
                case 7: return Action::Confirm;
                case 4: return Action::RotateLeft;
                case 5: return Action::RotateRight;
                default: return Action::None;
                }
            }

            if (event.type == sf::Event::JoystickMoved
                && event.joystickMove.joystickId == 0) {
                float pos = event.joystickMove.position;
                switch (event.joystickMove.axis) {
                case sf::Joystick::X:
                    if (pos > 50.f)  return Action::MoveRight;
                    if (pos < -50.f) return Action::MoveLeft;
                    break;
                case sf::Joystick::Y:
                    if (pos > 50.f)  return Action::MoveDown;
                    if (pos < -50.f) return Action::MoveUp;
                    break;
                case sf::Joystick::PovX:
                    if (pos > 50.f)  return Action::MoveRight;
                    if (pos < -50.f) return Action::MoveLeft;
                    break;
                case sf::Joystick::PovY:
                    if (pos > 50.f)  return Action::MoveUp;
                    if (pos < -50.f) return Action::MoveDown;
                    break;
                default: break;
                }
            }
        }

        return Action::None;
    }

}