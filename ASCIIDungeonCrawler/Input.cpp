#include "Input.h"
#include <conio.h>  
namespace DungeonGame {

    //arrow keys - two bite integers 
    //TODO: Add WASD as well 
    //might not be necessary 
    constexpr int KEY_EXTENDED = 224;
    constexpr int KEY_UP = 72;
    constexpr int KEY_DOWN = 80;
    constexpr int KEY_LEFT = 75;
    constexpr int KEY_RIGHT = 77;
    constexpr int KEY_SPACE = 32;
    constexpr int KEY_ESC = 27;
    constexpr int KEY_TAB = 9;

    Action getInput() {
        int ch = _getch();

        if (ch == KEY_ESC)   return Action::Quit;
        if (ch == KEY_SPACE) return Action::Interact;
        if (ch == KEY_TAB)   return Action::ToggleInventory;

        if (ch == KEY_EXTENDED) {
            int arrow = _getch();
            switch (arrow) {
            case KEY_UP:    return Action::MoveUp;
            case KEY_DOWN:  return Action::MoveDown;
            case KEY_LEFT:  return Action::MoveLeft;
            case KEY_RIGHT: return Action::MoveRight;
            }
        }

        return Action::None;
    }

}