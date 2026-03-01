#pragma once
#include "Entity.h"

namespace DungeonGame {

    class Merchant : public Entity {
    public:
        Merchant(int x, int y)
            : Entity(x, y, '~', EntityType::Merchant, "Merchant") {
        }

       //TODO: Trade - buying and selling
    };

}