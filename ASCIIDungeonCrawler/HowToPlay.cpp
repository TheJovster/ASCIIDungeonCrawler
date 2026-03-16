#include "HowToPlay.h"
#include <vector>
#include <string>

namespace DungeonGame {

    void showHowToPlay(sf::RenderWindow& window) {
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");

        struct Page {
            std::string title;
            std::vector<std::string> lines;
        };

        std::vector<Page> pages = {
            {
                "CONTROLS",
                {
                    "MOVEMENT",
                    "  W / S          Move forward / backward",
                    "  A / D          Strafe left / right",
                    "  Q / E          Rotate left / right",
                    "",
                    "ACTIONS",
                    "  Space          Interact with enemies, chests,",
                    "                 merchants and the exit",
                    "  Tab            Open / close inventory",
                    "  Esc            Open quit menu",
                    "",
                    "NAVIGATION",
                    "  Arrow keys     Navigate menus",
                    "  Y / N          Confirm / deny prompts",
                }
            },
            {
                "COMBAT",
            {
                    "TRIGGERING COMBAT",
                    "  Face an adjacent enemy and press Space.",
                    "  Alternatively, an enemy moving into",
                    "  your tile will strike first.",
                    "",
                    "TURN STRUCTURE",
                    "  Choose: Attack, Defend, Use Item or Flee.",
                    "  Defend reduces incoming damage this turn.",
                    "  Flee success depends on enemy tier.",
                    "  After your action, the enemy strikes back.",
                    "",
                    "DAMAGE & DEFEAT",
                    "  Attack vs Defense with diminishing returns.",
                    "  High defense never reduces damage to zero.",
                    "  Critical hits deal 1.5x damage (10% chance).",
                    "  HP reaches 0 — game over. No saving.",
            }
            },
            {
                "ITEMS & EQUIPMENT",
                {
                    "INVENTORY",
                    "  Press Tab to open your inventory.",
                    "  Starting capacity is 30 items.",
                    "  Merchants can upgrade your capacity.",
                    "",
                    "ITEM TYPES",
                    "  Equipment     Weapons, armor, shields.",
                    "                Equip to boost your stats.",
                    "  Consumables   Potions restore HP.",
                    "                Torches provide light.",
                    "  Vendor Trash  No use — sell for gold.",
                    "",
                    "EQUIPMENT SLOTS",
                    "  Head, Chest, Arms, Legs, Boots,",
                    "  Weapon, Shield, Torch.",
                    "",
                    "  Better weapons and armor drop on deeper floors.",
                    "  Adamantium is the rarest tier.",
                }
            },
            {
                "SURVIVAL",
                {
                    "TORCHES",
                    "  You start with one torch equipped.",
                    "  Each step burns one charge.",
                    "  When it runs out — darkness.",
                    "  Find or buy torches to survive.",
                    "",
                    "MERCHANTS",
                    "  Found in dungeon rooms.",
                    "  Buy items, sell loot, upgrade inventory.",
                    "  Stock is randomized each floor.",
                    "",
                    "THE EXIT",
                    "  A door marks the way out.",
                    "  Stand in front of it and press Space to descend.",
                    "  Enemies get stronger each floor.",
                    "  Loot improves too — keep pushing.",
                    "",
                    "  Good luck.",
                }
            }
        };

        int currentPage = 0;
        const int totalPages = (int)pages.size();

        auto makeTextCentered = [&](const std::string& str, float x, float y,
            sf::Color color = sf::Color::White,
            unsigned int size = 16) {
                sf::Text t;
                t.setFont(font);
                t.setCharacterSize(size);
                t.setFillColor(color);
                t.setString(str);
                sf::FloatRect bounds = t.getLocalBounds();
                t.setOrigin(bounds.width * 0.5f, 0.f);
                t.setPosition(x, y);
                return t;
            };

        auto makeText = [&](const std::string& str, float x, float y,
            sf::Color color = sf::Color::White,
            unsigned int size = 16) {
                sf::Text t;
                t.setFont(font);
                t.setCharacterSize(size);
                t.setFillColor(color);
                t.setString(str);
                t.setPosition(x, y);
                return t;
            };

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return;
                }
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                    case sf::Keyboard::Left:
                    case sf::Keyboard::A:
                        currentPage = std::max(0, currentPage - 1);
                        break;
                    case sf::Keyboard::Right:
                    case sf::Keyboard::D:
                        currentPage = std::min(totalPages - 1, currentPage + 1);
                        break;
                    case sf::Keyboard::Escape:
                    case sf::Keyboard::Backspace:
                        return;
                    default: break;
                    }
                }
            }

            window.clear(sf::Color::Black);

            // title
            window.draw(makeTextCentered("ASCII DUNGEON CRAWLER",
                (float)window.getSize().x * 0.5f, 40.f,
                sf::Color::Yellow, 22));

            window.draw(makeTextCentered(" - HOW TO PLAY - ",
                (float)window.getSize().x * 0.5f, 80.f,
                sf::Color::Yellow, 22));

            // page title
            window.draw(makeTextCentered(pages[currentPage].title,
                (float)window.getSize().x * 0.5f, 120.f,
                sf::Color::Cyan, 20));

            // divider
            window.draw(makeTextCentered(std::string(60, '-'),
                (float)window.getSize().x * 0.5f, 160.f,
                sf::Color(80, 80, 80)));

            // page content
            float lineY = 240.f;
            for (const auto& line : pages[currentPage].lines) {
                sf::Color color = sf::Color::White;
                // section headers — no leading spaces, not empty
                if (!line.empty() && line[0] != ' ')
                    color = sf::Color(255, 200, 50);
                window.draw(makeText(line, ((float)window.getSize().x * 0.5f) - 180.f, lineY, color));
                lineY += 26.f;
            }

            // page indicator
            std::string pageStr = "Page " + std::to_string(currentPage + 1)
                + " / " + std::to_string(totalPages);
            window.draw(makeText(pageStr,
                (float)window.getSize().x * 0.5f, (float)window.getSize().y - 80.f,
                sf::Color(150, 150, 150)));

            // navigation hint
            window.draw(makeText(
                "[A / Left]  Previous        [D / Right]  Next        [Esc]  Back",
                80.f, (float)window.getSize().y - 50.f,
                sf::Color(100, 100, 100)));

            window.display();
        }
    }

}