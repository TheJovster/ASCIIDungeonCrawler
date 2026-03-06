#include "Dungeon.h"
#include <random>
#include <algorithm>
#include "Enemy.h"
#include "Merchant.h"
#include "ItemDatabase.h"

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    template<typename T>
    static T randInt(T lo, T hi) {
        return std::uniform_int_distribution<T>(lo, hi)(rng);
    }

    Dungeon::Dungeon() {
        m_grid.assign(MAP_HEIGHT, std::vector<Tile>(MAP_WIDTH));
    }

    void Dungeon::generate(int floor) {
        for (auto& row : m_grid)
            for (auto& tile : row)
                tile = Tile{};

        m_rooms.clear();
        m_entities.clear();
        m_chests.clear();     

        placeRooms();
        carveCorridors();
        placeEntities(floor);
        placeExit();
        placeChests(floor);
    }

    void Dungeon::placeRooms() {
        constexpr int MAX_ATTEMPTS = 200;
        int attempts = 0;
        int targetRooms = randInt(MIN_ROOMS, MAX_ROOMS);

        while ((int)m_rooms.size() < targetRooms && attempts < MAX_ATTEMPTS) {
            ++attempts;

            Room r;
            r.width = randInt(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
            r.height = randInt(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
            r.x = randInt(1, MAP_WIDTH - r.width - 2);
            r.y = randInt(1, MAP_HEIGHT - r.height - 2);

            if (!roomOverlaps(r)) {
                m_rooms.push_back(r);
                carveRoom(r);
            }
        }
    }

    bool Dungeon::roomOverlaps(const Room& r) const {
        for (const auto& other : m_rooms) {
            if (r.x - 1 <= other.x + other.width &&
                r.x + r.width + 1 >= other.x &&
                r.y - 1 <= other.y + other.height &&
                r.y + r.height + 1 >= other.y)
                return true;
        }
        return false;
    }

    void Dungeon::carveRoom(const Room& r) {
        for (int row = r.y; row < r.y + r.height; ++row) {
            for (int col = r.x; col < r.x + r.width; ++col) {
                bool isTop = (row == r.y);
                bool isBottom = (row == r.y + r.height - 1);
                bool isLeft = (col == r.x);
                bool isRight = (col == r.x + r.width - 1);

                if (isTop || isBottom)
                    m_grid[row][col].type = TileType::WallH;
                else if (isLeft || isRight)
                    m_grid[row][col].type = TileType::WallV;
                else
                    m_grid[row][col].type = TileType::Floor;
            }
        }
    }

    void Dungeon::carveCorridors() {
        for (int i = 0; i + 1 < (int)m_rooms.size(); ++i) {
            int x1 = m_rooms[i].centerX(), y1 = m_rooms[i].centerY();
            int x2 = m_rooms[i + 1].centerX(), y2 = m_rooms[i + 1].centerY();

            if (randInt(0, 1) == 0) {
                carveHCorridor(x1, x2, y1);
                carveVCorridor(y1, y2, x2);
            }
            else {
                carveVCorridor(y1, y2, x1);
                carveHCorridor(x1, x2, y2);
            }
        }
    }

    void Dungeon::carveHCorridor(int x1, int x2, int y) {
        if (x1 > x2) std::swap(x1, x2);
        for (int x = x1; x <= x2; ++x)
            if (m_grid[y][x].type != TileType::Floor)
                m_grid[y][x].type = TileType::Floor;
    }

    void Dungeon::carveVCorridor(int y1, int y2, int x) {
        if (y1 > y2) std::swap(y1, y2);
        for (int y = y1; y <= y2; ++y)
            if (m_grid[y][x].type != TileType::Floor)
                m_grid[y][x].type = TileType::Floor;
    }

    void Dungeon::placeEntities(int floor) {
        int roomCount = (int)m_rooms.size();

        if (roomCount >= 3) {
            int merchantRoom = randInt(1, roomCount - 2);
            const Room& r = m_rooms[merchantRoom];
            m_entities.push_back(std::make_unique<Merchant>(r.centerX(), r.centerY(), floor));
        }

        for (int i = 1; i < roomCount - 1; ++i) {
            const Room& r = m_rooms[i];
            int count = randInt(1, 2);
            for (int j = 0; j < count; ++j) {
                int ex = randInt(r.x + 1, r.x + r.width - 2);
                int ey = randInt(r.y + 1, r.y + r.height - 2);
                EnemyTier tier = (EnemyTier)randInt(0, 2);
                auto enemy = std::make_unique<Enemy>(ex, ey, tier);
                enemy->scaleToFloor(floor);
                m_entities.push_back(std::move(enemy));
            }
        }
    }

    void Dungeon::placeExit() {
        const Room& last = m_rooms.back();
        m_exitX = last.centerX();
        m_exitY = last.centerY();
        m_grid[m_exitY][m_exitX].isExit = true;
    }

    void Dungeon::placeChests(int floor) {
        for (int i = 1; i < (int)m_rooms.size() - 1; ++i) {
            const Room& r = m_rooms[i];
            int cx = r.centerX() + 1;
            int cy = r.centerY();
            int key = cy * MAP_WIDTH + cx;

            std::vector<Item> items;
            int count = randInt(1, 3);
            for (int j = 0; j < count; ++j)
                items.push_back(ItemDatabase::get().randomItem(floor));

            m_chests[key] = items;
            m_grid[cy][cx].type = TileType::Floor;
        }
    }

    void Dungeon::removeEntity(Entity* entity) {
        m_entities.erase(
            std::remove_if(m_entities.begin(), m_entities.end(),
                [entity](const std::unique_ptr<Entity>& e) {
                    return e.get() == entity;
                }),
            m_entities.end()
        );
    }

}