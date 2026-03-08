#include "Renderer.h"
#include "Game.h"
#include "Enemy.h"
#include "InventorySystem.h"
#include "Item.h"
#include "Merchant.h"
#include <unordered_map>
#include <algorithm>

namespace DungeonGame {

    static char tileToChar(const Tile& tile) {
        switch (tile.type) {
        case TileType::Floor:  return '.';
        case TileType::WallH:  return '-';
        case TileType::WallV:  return '|';
        case TileType::Void:
        default:               return '#';
        }
    }

    Renderer::Renderer(sf::RenderWindow& window) {
        m_screenW = (int)window.getSize().x;
        m_screenH = (int)window.getSize().y;
        m_hudX = (int)(m_screenW * 0.62f);
        m_charSize = std::max(10, (int)(m_screenH / 720.f * 14.f));
        m_cellH = m_charSize + 2;
        m_cellW = m_charSize + 2;
        m_font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");
    }

    void Renderer::drawMap(sf::RenderWindow& window,
        const Dungeon& dungeon, const Player& player,
        const std::vector<std::string>& log,
        GameState state) const {

        const auto& grid = dungeon.getGrid();
        const auto& entities = dungeon.getEntities();
        const auto& chests = dungeon.getChests();

        // build entity symbol lookup
        std::unordered_map<int, char> entitySymbols;
        for (const auto& e : entities) {
            if (e->isAlive()) {
                int key = e->getY() * MAP_WIDTH + e->getX();
                entitySymbols[key] = e->getSymbol();
            }
        }

        auto drawChar = [&](int col, int row, char c, sf::Color color = sf::Color::White) {
            sf::Text text;
            text.setFont(m_font);
            text.setCharacterSize(CHAR_SIZE);
            text.setFillColor(color);
            text.setString(std::string(1, c));
            text.setPosition(
                (float)(col * m_cellW),
                (float)(row * m_cellH)
            );
            window.draw(text);
            };

        // draw map tiles
        for (int row = 0; row < MAP_HEIGHT; ++row) {
            for (int col = 0; col < MAP_WIDTH; ++col) {
                char      c = ' ';
                sf::Color color = sf::Color::White;

                if (row == player.y && col == player.x) {
                    c = '@';
                    color = sf::Color::Yellow;
                }
                else {
                    int key = row * MAP_WIDTH + col;
                    auto entityIt = entitySymbols.find(key);

                    if (entityIt != entitySymbols.end()) {
                        c = entityIt->second;
                        color = sf::Color::Red;
                    }
                    else {
                        auto chestIt = chests.find(key);
                        if (chestIt != chests.end()) {
                            c = chestIt->second.empty() ? 'c' : 'C';
                            color = sf::Color(255, 165, 0); // orange
                        }
                        else {
                            const Tile& tile = grid[row][col];
                            if (tile.isExit) {
                                c = '>';
                                color = sf::Color::Green;
                            }
                            else {
                                c = tileToChar(tile);
                                // dim walls, bright floor
                                color = (tile.type == TileType::Floor)
                                    ? sf::Color(160, 160, 160)
                                    : sf::Color(80, 80, 80);
                            }
                        }
                    }
                }
                drawChar(col, row, c, color);
            }
        }

        // separator line
        for (int col = 0; col < MAP_WIDTH; ++col)
            drawChar(col, MAP_HEIGHT, '-', sf::Color(80, 80, 80));

        // log lines
        int logSize = (int)log.size();
        int start = std::max(0, logSize - 3);
        for (int i = 0; i < 3; ++i) {
            std::string line = (start + i < logSize) ? log[start + i] : "";
            sf::Text text;
            text.setFont(m_font);
            text.setCharacterSize(CHAR_SIZE);
            text.setFillColor(sf::Color(200, 200, 200));
            text.setString(line);
            text.setPosition(0.f, (float)((MAP_HEIGHT + 1 + i) * m_cellH));
            window.draw(text);
        }
    }

    void Renderer::drawHUD(sf::RenderWindow& window,
        const Player& player, GameState state,
        const Enemy* activeEnemy, int floor,
        bool inventoryMode,
        const std::vector<Item>* chestContents,
        int chestSelected,
        int inventoryActionSelected,
        const Merchant* activeMerchant,
        MerchantMode merchantMode,
        int merchantTopSelected,
        int sellIndex,
        const CombatHUDData& combatData, 
        const std::string& dateString
        ) const {

        int row = 0;

        auto writeStr = [&](int r, const std::string& text,
            sf::Color color = sf::Color::White) {
                sf::Text t;
                t.setFont(m_font);
                t.setCharacterSize(m_charSize);
                t.setFillColor(color);
                t.setString(text);
                t.setPosition((float)m_hudX, (float)(r * m_cellH));
                window.draw(t);
            };

        std::string divider(30, '-');

        // --- player stats ---
        writeStr(row++, "PLAYER", sf::Color::Yellow);
        writeStr(row++, divider);
        writeStr(row++, "HP:  " + std::to_string(player.hp) + "/" + std::to_string(player.maxHP()));
        writeStr(row++, "ATK: " + std::to_string(player.attack()) + "  DEF: " + std::to_string(player.defense()));
        writeStr(row++, "Gold: " + std::to_string(player.gold), sf::Color(255, 215, 0));
        writeStr(row++, divider);

        // --- equipment slots ---
        auto slotStr = [&](const std::string& label, const std::optional<Item>& slot) {
            std::string val = slot.has_value() ? slot->name : "none";
            return "[" + label + "] " + val;
            };

        writeStr(row++, slotStr("Head  ", player.equipment.head));
        writeStr(row++, slotStr("Chest ", player.equipment.chest));
        writeStr(row++, slotStr("Arms  ", player.equipment.arms));
        writeStr(row++, slotStr("Legs  ", player.equipment.legs));
        writeStr(row++, slotStr("Boots ", player.equipment.boots));
        writeStr(row++, slotStr("Weapon", player.equipment.weapon));
        writeStr(row++, slotStr("Shield", player.equipment.shield));
        // torch slot — show charges if equipped
        if (player.equipment.torch.has_value()) {
            std::string torchStr = "[Torch ] " + player.equipment.torch->name
                + " (" + std::to_string(player.equipment.torch->charges) + ")";
            writeStr(row++, torchStr);
        }
        else {
            writeStr(row++, "[Torch ] none");
        }
        writeStr(row++, divider);

        // --- inventory ---
        const auto& inv = player.inventory;
        int         invCount = inv.count();
        int         invCap = inv.capacity();
        writeStr(row++, "INV: " + std::to_string(invCount) + "/" + std::to_string(invCap));

        int  selectedIndex = inv.getSelectedIndex();
        int  scrollOffset = inv.getScrollOffset();
        bool scrollable = inventoryMode;

        for (int i = 0; i < InventorySystem::VISIBLE_ROWS; ++i) {
            int itemIndex = scrollOffset + i;
            if (itemIndex >= invCount) { writeStr(row++, ""); continue; }

            const Item& item = inv.getItem(itemIndex);
            bool        selected = scrollable && (itemIndex == selectedIndex);

            std::string name = item.name;
            int maxNameLen = 25;
            if ((int)name.size() > maxNameLen)
                name = name.substr(0, maxNameLen);

            std::string line = name;
            if (selected) {
                while ((int)line.size() < maxNameLen) line += ' ';
                line += " <--";
            }
            writeStr(row++, line, selected ? sf::Color::Cyan : sf::Color::White);
        }

        writeStr(row++, divider);
        writeStr(row++, "FLOOR " + std::to_string(floor), sf::Color::Green);
        writeStr(row++, divider);

        // --- game state section ---
        switch (state) {
        case GameState::Exploring:
            writeStr(row++, inventoryMode ? "MODE: INVENTORY" : "MODE: EXPLORE", sf::Color(200, 200, 200));
            writeStr(row++, "[Tab]  Switch Mode");
            writeStr(row++, "[Esc]  Menu");
            writeStr(row++, " ");
            writeStr(row++, dateString,
                sf::Color(180, 180, 180));
            break;

            //combat game state
        case GameState::Combat:
            writeStr(row++, "COMBAT", sf::Color::Red);
            if (activeEnemy) {
                writeStr(row++, "vs " + activeEnemy->getName(), sf::Color::Red);
                writeStr(row++, "HP: " + std::to_string(activeEnemy->getHP()) + "/" +
                    std::to_string(activeEnemy->getMaxHP()));
            }
            else {
                writeStr(row++, "");
                writeStr(row++, "");
            }
            writeStr(row++, divider);

            switch (combatData.phase) {

            case CombatPhase::ActionSelect:
                writeStr(row++, combatData.actionSelected == 0 ? "> Attack" : "  Attack");
                writeStr(row++, combatData.actionSelected == 1 ? "> Defend" : "  Defend");
                writeStr(row++, combatData.actionSelected == 2 ? "> Use Item" : "  Use Item");
                writeStr(row++, combatData.actionSelected == 3 ? "> Flee" : "  Flee");
                writeStr(row++, divider);
                writeStr(row++, "[Up/Dn] Select");
                writeStr(row++, "[Space] Confirm");
                break;

            case CombatPhase::ItemSelect:
                if (combatData.itemList && combatData.playerInventory) {
                    if (combatData.itemList->empty()) {
                        writeStr(row++, "No consumables.", sf::Color(150, 150, 150));
                    }
                    else {
                        for (int i = 0; i < (int)combatData.itemList->size(); ++i) {
                            int idx = (*combatData.itemList)[i];
                            const Item& item = combatData.playerInventory->getItem(idx);
                            bool sel = (i == combatData.itemSelected);
                            std::string line = item.name;
                            if (sel) { while ((int)line.size() < 20) line += ' '; line += " <--"; }
                            writeStr(row++, line, sel ? sf::Color::Cyan : sf::Color::White);
                        }
                    }
                }
                writeStr(row++, divider);
                writeStr(row++, "[Space] Use");
                writeStr(row++, "[Esc]   Back");
                break;

            case CombatPhase::Resolution:
                if (combatData.lastAction && !combatData.lastAction->empty())
                    writeStr(row++, *combatData.lastAction, sf::Color::Yellow);
                else
                    writeStr(row++, "");
                writeStr(row++, divider);
                if (combatData.history) {
                    for (const auto& line : *combatData.history)
                        writeStr(row++, line, sf::Color(180, 180, 180));
                }
                writeStr(row++, divider);
                writeStr(row++, "[Space] Continue", sf::Color(150, 150, 150));
                break;
            }
            break;

        case GameState::InventoryAction: {
            if (player.inventory.count() > 0) {
                const Item& item = player.inventory.getItem(player.inventory.getSelectedIndex());
                bool        isEquip = item.type == ItemType::Equipment;
                bool        isConsume = item.type == ItemType::Consumable;
                if (isEquip) {
                    writeStr(row++, inventoryActionSelected == 0 ? "> Equip" : "  Equip");
                    writeStr(row++, inventoryActionSelected == 1 ? "> Drop" : "  Drop");
                }
                else if (isConsume) {
                    writeStr(row++, inventoryActionSelected == 0 ? "> Use" : "  Use");
                    writeStr(row++, inventoryActionSelected == 1 ? "> Drop" : "  Drop");
                }
                else {
                    writeStr(row++, inventoryActionSelected == 0 ? "> Drop" : "  Drop");
                    writeStr(row++, "");
                }
            }
            break;


        }

        case GameState::ChestLoot:
            writeStr(row++, "CHEST", sf::Color(255, 165, 0));
            writeStr(row++, divider);
            if (chestContents && !chestContents->empty()) {
                for (int i = 0; i < (int)chestContents->size(); ++i) {
                    const Item& item = (*chestContents)[i];
                    std::string line = item.name;
                    if (i == chestSelected) {
                        while ((int)line.size() < 25) line += ' ';
                        line += " <--";
                    }
                    writeStr(row++, line, i == chestSelected ? sf::Color::Cyan : sf::Color::White);
                }
            }
            else {
                writeStr(row++, "Empty.");
            }
            writeStr(row++, divider);
            writeStr(row++, "[Space] Take");
            writeStr(row++, "[Up/Dn] Browse");
            writeStr(row++, "[Esc]   Close");
            break;

        case GameState::GameOver:{
            writeStr(row++, "GAME OVER", sf::Color::Red);
            writeStr(row++, divider);
            writeStr(row++, "Floor " + std::to_string(floor));
            writeStr(row++, "");
            writeStr(row++, "[Space] Continue", sf::Color(150, 150, 150));
            int maxRows = m_screenH / m_cellH;
            while (row < maxRows) writeStr(row++, "");
            break;
        }

        case GameState::MerchantMenu: {
            if (!activeMerchant) break;
            switch (merchantMode) {
            case MerchantMode::TopMenu: {
                int upgradeCost = 150 * (1 << ((player.inventory.capacity() - 30) / 10));
                std::string upgradeLabel = player.inventory.capacity() >= 80
                    ? "Upgrade (MAX)"
                    : "Upgrade (" + std::to_string(upgradeCost) + "g)";
                writeStr(row++, "MERCHANT", sf::Color(255, 165, 0));
                writeStr(row++, divider);
                writeStr(row++, merchantTopSelected == 0 ? "> Buy" : "  Buy");
                writeStr(row++, merchantTopSelected == 1 ? "> Sell" : "  Sell");
                writeStr(row++, merchantTopSelected == 2 ? "> " + upgradeLabel : "  " + upgradeLabel);
                writeStr(row++, merchantTopSelected == 3 ? "> Leave" : "  Leave");
                writeStr(row++, divider);
                writeStr(row++, "[Up/Dn] Select");
                writeStr(row++, "[Space] Confirm");
                writeStr(row++, "[Esc]   Leave");
                break;
            }
            case MerchantMode::Buy: {
                writeStr(row++, "BUY", sf::Color(255, 215, 0));
                writeStr(row++, divider);
                const auto& stock = activeMerchant->getStock();
                int         sel = activeMerchant->getSelectedIndex();
                if (stock.empty()) {
                    writeStr(row++, "Out of stock.");
                    for (int i = 0; i < 5; ++i) writeStr(row++, "");
                }
                else {
                    int total = (int)stock.size();
                    int start = std::max(0, sel - 2);
                    int end = std::min(total, start + 6);
                    start = std::max(0, end - 6);
                    for (int i = 0; i < 6; ++i) {
                        int idx = start + i;
                        if (idx >= total) { writeStr(row++, ""); continue; }
                        const Item& item = stock[idx];
                        int         price = activeMerchant->buyPrice(item);
                        std::string line = item.name + " " + std::to_string(price) + "g";
                        bool        sel_ = (idx == sel);
                        if (sel_) {
                            while ((int)line.size() < 25) line += ' ';
                            line += " <--";
                        }
                        writeStr(row++, line, sel_ ? sf::Color::Cyan : sf::Color::White);
                    }
                }
                writeStr(row++, divider);
                writeStr(row++, "[Space] Buy");
                writeStr(row++, "[Esc]   Back");
                break;
            }
            case MerchantMode::Sell: {
                writeStr(row++, "SELL", sf::Color(255, 215, 0));
                writeStr(row++, divider);
                const auto& inv = player.inventory;
                if (inv.count() == 0) {
                    writeStr(row++, "Nothing to sell.");
                    for (int i = 0; i < 5; ++i) writeStr(row++, "");
                }
                else {
                    int sel = sellIndex;
                    int total = inv.count();
                    int start = std::min(sel, std::max(0, total - MERCHANT_VISIBLE_ROWS));
                    for (int i = 0; i < MERCHANT_VISIBLE_ROWS; ++i) {
                        int idx = start + i;
                        if (idx >= total) { writeStr(row++, ""); continue; }
                        const Item& item = inv.getItem(idx);
                        int         price = activeMerchant->sellPrice(item);
                        std::string line = item.name + " " + std::to_string(price) + "g";
                        bool        sel_ = (idx == sel);
                        if (sel_) {
                            while ((int)line.size() < 25) line += ' ';
                            line += " <--";
                        }
                        writeStr(row++, line, sel_ ? sf::Color::Cyan : sf::Color::White);
                    }
                }
                writeStr(row++, divider);
                writeStr(row++, "[Space] Sell");
                writeStr(row++, "[Esc]   Back");
                break;
            }
            }
            break;
        }

        case GameState::ExitPrompt:
            writeStr(row++, "DESCEND?", sf::Color::Green);
            writeStr(row++, divider);
            writeStr(row++, "Floor " + std::to_string(floor) + " -> " + std::to_string(floor + 1));
            writeStr(row++, "");
            writeStr(row++, "[Y] Descend");
            writeStr(row++, "[N] Stay");
            break;

        case GameState::QuitPrompt:
            writeStr(row++, "QUIT TO MENU?", sf::Color::Red);
            writeStr(row++, divider);
            writeStr(row++, "[Y] Yes");
            writeStr(row++, "[N] No");
            break;
        }
    }
}