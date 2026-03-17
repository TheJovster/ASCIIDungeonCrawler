#pragma once
#include <SFML/Graphics.hpp>
#include "HowToPlay.h"

namespace DungeonGame {

    constexpr float MENU_BUTTON_WIDTH_PCT = 0.18f;  // % of screen width
    constexpr float MENU_BUTTON_GAP = 15.f;   // px between buttons

    bool showMainMenu(sf::RenderWindow& window);

}