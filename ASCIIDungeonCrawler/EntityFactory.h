#pragma once
#include "Enemy.h"
#include "Merchant.h"
#include <memory>

namespace DungeonGame 
{
	class EntityFactory
	{
	public:

		static std::unique_ptr<Enemy> createEnemy(int x, int y, EnemyType tier, int floor);
		static std::unique_ptr<Merchant> createMerchant(int x, int y, int floor);
	};
}

