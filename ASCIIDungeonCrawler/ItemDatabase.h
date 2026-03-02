#pragma once
#include "Item.h"
#include <vector>

namespace DungeonGame {

    class ItemDatabase {
    public:
        static const ItemDatabase& get();
        Item randomConsumable()  const;
        Item randomEquipment()   const;
        Item randomVendorTrash() const;
        Item randomItem()        const;

    private:
        ItemDatabase();
        std::vector<Item> m_consumables;
        std::vector<Item> m_equipment;
        std::vector<Item> m_vendorTrash;
    };

}