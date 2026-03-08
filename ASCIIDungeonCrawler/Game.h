#pragma once
#include "Dungeon.h"
#include "Renderer.h"
#include "Player.h"
#include "Input.h"
#include "CombatSystem.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "AudioManager.h"
#include "RaycastRenderer.h"
#include "GameTime.h"

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
       
        void run(sf::RenderWindow& window);
        const GameTime& getTime() { return m_time; }

    private:
        static constexpr float PI = 3.14159265f;

        Dungeon         m_dungeon;
        Renderer        m_renderer;
        RaycastRenderer m_raycastRenderer;
        Player          m_player;
        CombatSystem    m_combat;
        GameState       m_state = GameState::Exploring;
        Enemy*          m_activeEnemy = nullptr;
        GameTime        m_time;
        bool            m_running = true;
        bool            m_inventoryMode = false;
        bool            m_pendingEnemyTurn = false;
        bool            m_pendingCombatEnd = false;
        int             m_floor = 1;

        int             m_chestKey = -1;   // key of active chest
        int             m_chestSelected = 0;    // selected item index in chest

        int             m_inventoryActionSelected = 0; // 0 = first option, 1 = second

        Merchant*       m_activeMerchant = nullptr;
        MerchantMode    m_merchantMode = MerchantMode::TopMenu;
        int             m_merchantTopSelected = 0; // 0=Buy, 1=Sell, 2=Leave

        CombatPhase          m_combatPhase = CombatPhase::ActionSelect;
        int                  m_combatActionSelected = 0;
        int                  m_combatItemSelected = 0;
        std::vector<int>     m_combatItemList;

        void resolveCombatTurn(CombatAction action);
        std::vector<int> buildCombatItemList();
        


        std::vector<std::string> m_log;
        int m_sellIndex = 0;

        void spawnPlayer();
        void handleExploring(Action action);
        void handleCombat(Action action);
        void handleGameOver(Action action);
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

        bool isPlayerFacing() const;

        int  getChestKeyAdjacent(int x, int y) const;
        Enemy* getEnemyAdjacent(int x, int y) const;
        bool isAdjacentToExit(int x, int y) const;

        void handleExitPrompt(Action action);
        void nextFloor();

        void updateEnemyPatrol();
    };

}