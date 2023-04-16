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
		detectEnemyBuildings();
		detectEnemyUnits();
	}
}

void ScoutManager::updateLastKnownEnemyUnitLocations()
{
	for (auto& e : lastKnownEnemyUnitLocations)
	{
		BWAPI::Unit enemy = e.first;
		// if enemy is visible again update its location
		if (enemy->isVisible())
		{
			lastKnownEnemyUnitLocations[enemy] = enemy->getPosition();
			lastKnownEnemyUnitNames[enemy] = enemy->getType().getName();
		}
	}
}

void ScoutManager::drawCirclesInMiniMap()
{
	for (const auto & e : lastKnownEnemyUnitLocations)
	{
		BWAPI::Unit enemy = e.first;
		BWAPI::Position position = e.second;
		// drawing a red circle with radious 10
		BWAPI::Broodwar->drawCircleMap(position, 10, BWAPI::Colors::Red);
		// printing the unit type near the circle
		std::string unitType = lastKnownEnemyUnitNames[enemy];
		int xOffset = (-unitType.length() * 2) - 2 ;
		int yOffset = 7; 
		BWAPI::Position textPosition = position + BWAPI::Position(xOffset, yOffset);
		BWAPI::Broodwar->drawTextMap(textPosition, "%s", unitType.c_str());
	}
}

void ScoutManager::detectEnemyBuildings()
{
	// detects enemy buildings and records last position of all units
	BWAPI::Unitset enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
	for (auto& unit : enemyUnits) {
		lastKnownEnemyUnitLocations[unit] = unit->getPosition();
		lastKnownEnemyUnitNames[unit] = unit->getType().getName();
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

void ScoutManager::detectEnemyUnits()
{
	if (scout == nullptr)
	{
		return;
	}
	BWAPI::Unitset nearbyUnits = BWAPI::Broodwar->getUnitsInRadius(scout->getPosition(), 500);

	for (auto& unit : nearbyUnits)
	{
		if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		{
			lastKnownEnemyUnitLocations[unit] = unit->getPosition();
			lastKnownEnemyUnitNames[unit] = unit->getType().getName();
			if (unit->getType().canAttack() || unit->getType().isSpellcaster())
			{
				// retreat scout
			}
		}
	}

	// return false;
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
	updateLastKnownEnemyUnitLocations();
	drawCirclesInMiniMap();
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

std::map<BWAPI::Unit, BWAPI::Position> ScoutManager::getLastKnownEnemyUnitLocations()
{
	return lastKnownEnemyUnitLocations;
}
