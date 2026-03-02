#include "InventorySystem.h"
#include <algorithm>

namespace DungeonGame {

    bool InventorySystem::addItem(const Item& item) {
        if (isFull()) return false;
        m_items.push_back(item);
        return true;
    }

    void InventorySystem::removeItem(int index) {
        if (index < 0 || index >= (int)m_items.size()) return;
        m_items.erase(m_items.begin() + index);
        clampScroll();
    }

    void InventorySystem::scrollUp() {
        if (m_selectedIndex > 0) {
            --m_selectedIndex;
            if (m_selectedIndex < m_scrollOffset)
                --m_scrollOffset;
        }
    }

    void InventorySystem::scrollDown() {
        if (m_selectedIndex < (int)m_items.size() - 1) {
            ++m_selectedIndex;
            if (m_selectedIndex >= m_scrollOffset + VISIBLE_ROWS)
                ++m_scrollOffset;
        }
    }

    void InventorySystem::clampScroll() {
        m_selectedIndex = std::min(m_selectedIndex, (int)m_items.size() - 1);
        m_selectedIndex = std::max(0, m_selectedIndex);
        m_scrollOffset = std::min(m_scrollOffset, (int)m_items.size() - 1);
        m_scrollOffset = std::max(0, m_scrollOffset);
    }

}