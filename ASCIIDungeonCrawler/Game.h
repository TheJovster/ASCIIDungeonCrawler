#pragma once
#include "Dungeon.h"
#include "Renderer.h"
#include "Player.h"
#include "Input.h"
#include "CombatSystem.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace DungeonGame {

    enum class GameState {
        Exploring,
        Combat,
        ChestLoot,
        InventoryAction,
        MerchantMenu,
        ExitPrompt,
        QuitPrompt,
        GameOver
    };

    class Game {
    public:
        Game();
        ~Game() { m_music.stop(); } //dtor - stops the music track
        void run(sf::RenderWindow& window);

    private:
        Dungeon      m_dungeon;
        Renderer     m_renderer;
        Player       m_player;
        CombatSystem m_combat;
        GameState    m_state = GameState::Exploring;
        Enemy* m_activeEnemy = nullptr;
        bool         m_running = true;
        bool m_inventoryMode = false;
        int m_floor = 1;

        int  m_chestKey = -1;   // key of active chest
        int  m_chestSelected = 0;    // selected item index in chest

        int m_inventoryActionSelected = 0; // 0 = first option, 1 = second

        Merchant* m_activeMerchant = nullptr;
        MerchantMode m_merchantMode = MerchantMode::TopMenu;
        int          m_merchantTopSelected = 0; // 0=Buy, 1=Sell, 2=Leave

        std::vector<std::string> m_log;
        int m_sellIndex = 0;

        sf::Music m_music;

        void spawnPlayer();
        void handleExploring(Action action);
        void handleCombat(Action action);
        void endCombat();
        bool isWalkable(int x, int y) const;
        Enemy* getEnemyAt(int x, int y) const;

        void handleChestLoot(Action action);
        int  getChestKeyAt(int x, int y) const;

        void handleInventoryAction(Action action);
        void handleQuitPrompt(Action action);

        void      handleMerchantMenu(Action action);
        Merchant* getMerchantAt(int x, int y) const;
        bool      isAdjacentToMerchant(int x, int y) const;

        void handleExitPrompt(Action action);
        void nextFloor();

        void updateEnemyPatrol();
    };

}