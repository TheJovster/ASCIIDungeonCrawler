#include "Merchant.h"
#include "ItemDatabase.h"
#include <random>

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    Merchant::Merchant(int x, int y, int floor)
        : Entity(x, y, '~', EntityType::Merchant, "Merchant") {
        generateStock(floor);
    }

    void Merchant::generateStock(int floor) {
        m_stock.clear();
        int count = std::uniform_int_distribution<int>(6, 12)(rng);
        for (int i = 0; i < count; ++i)
            m_stock.push_back(ItemDatabase::get().randomItem(floor));
    }

    void Merchant::removeStockItem(int index) {
        if (index < 0 || index >= (int)m_stock.size()) return;
        m_stock.erase(m_stock.begin() + index);
        if (m_selectedIndex >= (int)m_stock.size() && m_selectedIndex > 0)
            --m_selectedIndex;
    }

}