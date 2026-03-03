#pragma once
#include "Item.h"
#include <vector>
#include <optional>

namespace DungeonGame {

    class InventorySystem {
    public:
        static constexpr int DEFAULT_CAPACITY = 30;

        bool        addItem(const Item& item);
        void        removeItem(int index);
        bool        isFull()  const { return (int)m_items.size() >= m_capacity; }
        int         count()   const { return (int)m_items.size(); }
        int         capacity()const { return m_capacity; }
        void        upgradeCapacity(int amount) { m_capacity += amount; }

        const std::vector<Item>& getItems() const { return m_items; }
        const Item& getItem(int index)      const { return m_items[index]; }

        // scroll state
        int  getScrollOffset() const { return m_scrollOffset; }
        int  getSelectedIndex()const { return m_selectedIndex; }
        void scrollUp();
        void scrollDown();
        void resetScroll() { m_selectedIndex = 0; m_scrollOffset = 0; }

        static constexpr int VISIBLE_ROWS = 8;
        static constexpr int SCROLL_THRESHOLD = 6;

    private:
        std::vector<Item> m_items;
        int m_capacity = DEFAULT_CAPACITY;
        int m_scrollOffset = 0;
        int m_selectedIndex = 0;

        void clampScroll();
    };

}