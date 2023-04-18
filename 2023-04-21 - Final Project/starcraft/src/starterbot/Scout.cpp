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
		// detectEnemyBuildings();
		// detectEnemyUnits();
		// std::cout << lastKnownEnemyUnitLocations.size() << std::endl;
	}
}

void ScoutManager::updateLastKnownEnemyUnitLocations()
{
	for (auto& e : lastKnownEnemyUnitLocations)
	{
		int enemyID = e.first;
		BWAPI::Unit enmyunit = BWAPI::Broodwar->getUnit(enemyID);
		// if enemy is visible again update its location
		if (enmyunit->isVisible())
		{
			lastKnownEnemyUnitLocations[enemyID] = enmyunit->getPosition();
			lastKnownEnemyUnitNames[enemyID] = enmyunit->getType().getName();
		}
	}
}

void ScoutManager::drawCirclesInMiniMap()
{
	for (const auto & e : lastKnownEnemyUnitLocations)
	{
		int enemyID = e.first;
		BWAPI::Position position = e.second;
		// drawing a red circle with radious 10
		BWAPI::Broodwar->drawCircleMap(position, 10, BWAPI::Colors::Red);
		// printing the unit type near the circle
		std::string unitType = lastKnownEnemyUnitNames[enemyID];
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
		int id = unit->getID();
		lastKnownEnemyUnitLocations[id] = unit->getPosition();
		lastKnownEnemyUnitNames[id] = unit->getType().getName();
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
	BWAPI::Unitset nearbyUnits = BWAPI::Broodwar->getAllUnits();

	for (auto& unit : nearbyUnits)
	{
		if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		{
			int id = unit->getID();
			lastKnownEnemyUnitLocations[id] = unit->getPosition();
			lastKnownEnemyUnitNames[id] = unit->getType().getName();
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

			if (unit->getType().canAttack() || unit->getType().isSpellcaster())
			{
				scoutStatus = "retreat";
			}
			else 
			{
				scout->attack(unit);
			}
		}
	}

	// return false;
}

void ScoutManager::retreatScout()
{
	BWAPI::TilePosition startingBaseTilePosition = BWAPI::Broodwar->self()->getStartLocation();
	BWAPI::Position startingBasePosition = BWAPI::Position(startingBaseTilePosition);
	scout->move(startingBasePosition);
	checkIfScoutIsAtBase();
}

void ScoutManager::checkIfScoutIsAtBase()
{
	BWAPI::TilePosition startingBaseTilePosition = BWAPI::Broodwar->self()->getStartLocation();
	BWAPI::Position startingBasePosition = BWAPI::Position(startingBaseTilePosition);
	double distance = scout->getPosition().getApproxDistance(startingBasePosition);

	// checking if scout is within 200 units of base
	if (distance < 200)
	{
		scoutStatus = "None";
	}
}

void ScoutManager::scoutRoam()
{
	int playerQuadrant = getPlayerMapPositionByQuadrent();
	if (enemyFound)
	{
		scout->move(BWAPI::Position(enemyLocation));
	}
	else
	{
		int startX, endX, stepX;
		int startY, endY, stepY;
		int stepSize = 5;
		switch (playerQuadrant)
		{
		case 1:
			startX = map->width() - 1;
			endX = 0;
			stepX = -1 * stepSize;
			startY = map->height() - 1;
			endY = 0;
			stepY = -1 * stepSize;
			break;
		case 2:
			startX = 0;
			endX = map->width() - 1;
			stepX = stepSize;
			startY = map->height() - 1;
			endY = 0;
			stepY = -1 * stepSize;
			break;
		case 3:
			startX = map->width() - 1;
			endX = 0;
			stepX = -1 * stepSize;
			startY = 0;
			endY = map->height() - 1;
			stepY = stepSize;
			break;
		case 4:
		default:
			startX = 0;
			endX = map->width() - 1;
			stepX = stepSize;
			startY = 0;
			endY = map->height() - 1;
			stepY = stepSize;
			break;
		}

		traverseMap(startX, endX, stepX, startY, endY, stepY);

	}
}

void ScoutManager::traverseMap(int startX, int endX, int stepX, int startY, int endY, int stepY)
{
	bool foundNextTarget = false;
	for (int x = startX; (stepX > 0) ? x <= endX : x >= endX; x += stepX)
	{
		if (foundNextTarget)
		{
			break;
		}
		for (int y = startY; (stepY > 0) ? y <= endY : y >= endY; y += stepY)
		{
			std::string locationEncoding = std::to_string(x) + std::to_string(y);
			bool isWalkable = map->isWalkable(x, y);
			bool explored = map->isExplored(x, y);
			int mapVal = scoutMap->get(x, y);
			if (isWalkable && (!explored && (mapVal < 700)))
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

int ScoutManager::getPlayerMapPositionByQuadrent()
{
	int mapWidth = BWAPI::Broodwar->mapWidth() * 32;
	int mapHeight = BWAPI::Broodwar->mapHeight() * 32;

	// Calculate the center of the map
	int centerX = mapWidth / 2;
	int centerY = mapHeight / 2;

	BWAPI::Position startingBasePosition = BWAPI::Position(globalManager->playerLocation);

	// Determine the quadrant where the player's starting base is located
	if (startingBasePosition.x < centerX && startingBasePosition.y < centerY)
	{
		return 1;
	}
	else if (startingBasePosition.x >= centerX && startingBasePosition.y < centerY)
	{
		return 2;
	}
	else if (startingBasePosition.x < centerX && startingBasePosition.y >= centerY)
	{
		return 3;
	}
	else 
	{
		// startingBasePosition.x >= centerX && startingBasePosition.y >= centerY
		return 4;
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
	detectEnemyUnits();
	drawCirclesInMiniMap();
	if (scout)
	{
		if ((scoutStatus == "None") || (scoutStatus == "exploring"))
		{
			scoutRoam();
		}
		else if (scoutStatus == "retreat")
		{
			retreatScout();
		}
		else if (scoutStatus == "harass")
		{
			// harass code here
		}
		if (!scout->exists())
		{
			scoutStatus = "None";
			scout == nullptr;
		}
	}
	else if (!enemyFound)
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

std::map<int, BWAPI::Position> ScoutManager::getLastKnownEnemyUnitLocations()
{
	return lastKnownEnemyUnitLocations;
}
