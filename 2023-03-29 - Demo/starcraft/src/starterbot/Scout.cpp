#include "Scout.h"



void ScoutManager::checkIfEnemyFound()
{
	if (enemyFound)
	{
		BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 30), "Enemy Found!");
		std::string locationStr = "Location: " + std::to_string(enemyLocation.x) + ", " + std::to_string(enemyLocation.y);
		BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 40), locationStr.c_str());
	}
	else
	{
		// detects enemy buildings 
		BWAPI::Unitset enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
		for (auto& unit : enemyUnits) {
			if (unit->getType().isBuilding()) {
				BWAPI::Position ePos = unit->getPosition();
				enemyLocation = BWAPI::TilePosition(ePos);
				enemyFound = true;
				globalManager->enemyFound = true;
				globalManager->scoutStatus = "enemy_found";
				globalManager->enemyLocation = BWAPI::TilePosition(ePos);
				scoutStatus = "enemy_found";
				// (not needed as of now) todo: narrow down search space and do more enemy base exploration
			}
		}
	}
}

ScoutManager::ScoutManager(MapTools* mapInstance, GlobalManager* globalManagerInstance)
{
	map = mapInstance;
	globalManager = globalManagerInstance;
	enemy = BWAPI::Broodwar->enemy();
	scoutMap = new Grid(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight(), 0);
	std::cout << "ScoutManager Initialized." << std::endl;
}

void ScoutManager::update()
{
	checkIfEnemyFound();
	if (scout)
	{
		if ((scoutStatus == "None") || (scoutStatus == "exploring"))
		{
			bool foundNextTarget = false;
			bool noScoutLocationFound = false;
			if (enemyFound)
			{
				scout->move(BWAPI::Position(enemyLocation));
			}
			else
			{
				for (int x = 0; x < map->width(); x += 15)
				{
					if (foundNextTarget)
					{
						break;
					}
					for (int y = 0; y < map->height(); y += 15)
					{
						std::string locationEncoding = std::to_string(x) + std::to_string(y);
						bool isWalkable = map->isWalkable(x, y);
						bool explored = map->isExplored(x, y);
						int mapVal = scoutMap->get(x, y);
						if (isWalkable && (!explored && (mapVal < 500)))
						{
							exploringHash[locationEncoding] = true;
							scout->move(BWAPI::Position(BWAPI::TilePosition(x, y)));
							scoutStatus = "exploring";
							scoutMap->set(x, y, mapVal + 1);
							foundNextTarget = true;
							break;
						}
					}
				}
			}
			

			/*if (noScoutLocationFound)
			{
				scoutLevel++;
			}*/
		}
		if (!scout->exists())
		{
			scoutStatus = "None";
			scout == nullptr;
		}
	}
	else if (scoutStatus != "enemy_found")
	{
		scoutStatus = "None";
		scout == nullptr;
	}
}

BWAPI::Unit ScoutManager::getScout()
{
	return scout;
}

void ScoutManager::setScout(BWAPI::Unit unit)
{
	scout = unit;
}

BWAPI::TilePosition ScoutManager::getEnemyLocation()
{
	return enemyLocation;
}

std::string ScoutManager::getScoutStatus()
{
	return scoutStatus;
}
