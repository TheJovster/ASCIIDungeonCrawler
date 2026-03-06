#pragma once
#include "Entity.h"
#include "Item.h"
#include <vector>

namespace DungeonGame {

    class Merchant : public Entity {
    public:
        Merchant(int x, int y, int floor);
        void generateStock(int floor);

        const std::vector<Item>& getStock()        const { return m_stock; }
        int                      getSelectedIndex() const { return m_selectedIndex; }
        void                     setSelectedIndex(int i) { m_selectedIndex = i; }

        static constexpr float BUY_MARKUP = 1.5f;
        static constexpr float SELL_MARGIN = 0.5f;

        int buyPrice(const Item& item)  const { return (int)(item.value * BUY_MARKUP); }
        int sellPrice(const Item& item) const { return (int)(item.value * SELL_MARGIN); }

        void removeStockItem(int index);

        std::string getTextureName() const override {
            return "assets/texture_merchant.png";
        }

    private:
        std::vector<Item> m_stock;
        int               m_selectedIndex = 0;

    protected:

    };

}