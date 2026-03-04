#pragma once
#include "Tile.h"
#include "Enemy.h"
#include "Merchant.h"
#include "Item.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace DungeonGame {

    struct Room {
        int x, y;
        int width, height;

        int centerX() const { return x + width / 2; }
        int centerY() const { return y + height / 2; }
    };

    class Dungeon {
    public:
        Dungeon();
        void generate(int floor = 1);

        const std::vector<std::vector<Tile>>& getGrid()     const { return m_grid; }
        const std::vector<Room>& getRooms()    const { return m_rooms; }
        const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }
        const std::unordered_map<int, std::vector<Item>>& getChests() const { return m_chests; }
        std::unordered_map<int, std::vector<Item>>& getChests() { return m_chests; }

        void removeEntity(Entity* entity);
        std::vector<std::unique_ptr<Entity>>& getEntities() { return m_entities; }

    private:
        std::vector<std::vector<Tile>>               m_grid;
        std::vector<Room>                            m_rooms;
        std::vector<std::unique_ptr<Entity>>         m_entities;
        std::unordered_map<int, std::vector<Item>>   m_chests;

        void placeRooms();
        void carveRoom(const Room& room);
        void carveCorridors();
        void carveHCorridor(int x1, int x2, int y);
        void carveVCorridor(int y1, int y2, int x);
        bool roomOverlaps(const Room& room) const;

        void placeEntities(int floor);
        void placeExit();
        void placeChests(int floor);

    };

}