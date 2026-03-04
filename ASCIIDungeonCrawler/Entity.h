#pragma once
#include <string>

namespace DungeonGame {

    enum class EntityType {
        Enemy,
        Merchant
    };

    class Entity {
    public:
        Entity(int x, int y, char symbol, EntityType type, const std::string& name)
            : m_x(x), m_y(y), m_symbol(symbol), m_type(type), m_name(name) {
        }

        virtual ~Entity() = default;

        int         getX()      const { return m_x; }
        int         getY()      const { return m_y; }
        char        getSymbol() const { return m_symbol; }
        EntityType  getType()   const { return m_type; }
        const std::string& getName() const { return m_name; }

        bool isAlive() const { return m_alive; }
        void kill() { m_alive = false; }
        void setPosition(int x, int y) { m_x = x; m_y = y; }

    protected:
        int        m_x = 0;
        int        m_y = 0;
        char       m_symbol = '?';
        EntityType m_type;
        std::string m_name;
        bool       m_alive = true;

    };

}