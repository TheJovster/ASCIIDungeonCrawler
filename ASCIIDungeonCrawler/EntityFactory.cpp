#include "EntityFactory.h"

namespace DungeonGame 
{
	std::unique_ptr<Enemy> EntityFactory::createEnemy(int x, int y, EnemyType tier, int floor)
	{
		auto enemy = std::make_unique<Enemy>(x, y, tier);
		enemy->scaleToFloor(floor);
		return enemy;
	}

	std::unique_ptr<Merchant> EntityFactory::createMerchant(int x, int y, int floor)
	{

		return std::make_unique<Merchant>(x, y, floor);
	}
}


