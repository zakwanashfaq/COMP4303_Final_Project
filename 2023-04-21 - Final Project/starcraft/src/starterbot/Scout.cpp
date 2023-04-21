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
			calculateBestPlaceForExapnsion();
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
				calculateBestPlaceForExapnsion();
			}
		}
	}
	BWAPI::Unitset scoutNearbyUnits = BWAPI::Broodwar->getUnitsInRadius(scout->getPosition(), 9700);
	for (auto& unit : scoutNearbyUnits)
	{
		if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		{
			if (scout && scout->exists() && scout->isUnderAttack())
			{
				scoutStatus = "retreat";
			}
			else
			{
				scout->attack(unit);
			}
		}
	}
}

void ScoutManager::detectChokePoint()
{
	BWAPI::TilePosition playerLocation = globalManager->playerLocation;
	BWAPI::TilePosition enemyLocation = globalManager->enemyLocation;

	// openList and closed list
	std::priority_queue<std::shared_ptr<TileNode>, std::vector<std::shared_ptr<TileNode>>, CompareNode> openList;
	std::vector<std::shared_ptr<TileNode>> closedList;

	TileNode startNode(playerLocation, 0, 0, NULL);
	openList.push(std::make_shared<TileNode>(startNode));
	std::vector<BWAPI::TilePosition> path;
	bool goalFound = false;

	while ((openList.size() > 0) && !goalFound)
	{
		// add node to closed list and pop node from open list and store it in a variable
		std::shared_ptr<TileNode> currentNode = openList.top();
		openList.pop();
		closedList.push_back(currentNode);

		// find path to goal node (found goal location)
		if (currentNode->tile == enemyLocation)
		{
			goalFound = true;
			std::shared_ptr<TileNode> currentNode = closedList.back();
			while (currentNode)
			{
				path.push_back(currentNode->tile);
				// converting raw pointer to shared pointer
				std::shared_ptr<TileNode> tempNode(currentNode->parent);
				currentNode = tempNode;
			}
			std::reverse(path.begin(), path.end());
			break;
		}

		// expand each node, check if it's valid
		// if valid find state evaluation value by using evaluateTileNode
		// then add to openList
		// up node
		std::shared_ptr<TileNode> upNode = std::make_shared<TileNode>(
			BWAPI::TilePosition(currentNode->tile.x, currentNode->tile.y - 1),
			currentNode->cost + 1,
			evaluateTileNode(upNode),
			currentNode.get()
		);
		if (isValidAndBuildable(upNode->tile))
		{
			openList.push(upNode);
		}

		// down node
		std::shared_ptr<TileNode> downNode = std::make_shared<TileNode>(
			BWAPI::TilePosition(currentNode->tile.x, currentNode->tile.y + 1),
			currentNode->cost + 1,
			evaluateTileNode(downNode),
			currentNode.get()
		);
		if (isValidAndBuildable(downNode->tile))
		{
			openList.push(downNode);
		}

		// left node
		std::shared_ptr<TileNode> leftNode = std::make_shared<TileNode>(
			BWAPI::TilePosition(currentNode->tile.x - 1, currentNode->tile.y),
			currentNode->cost + 1,
			evaluateTileNode(leftNode),
			currentNode.get()
		);
		if (isValidAndBuildable(leftNode->tile))
		{
			openList.push(leftNode);
		}

		// right node
		std::shared_ptr<TileNode> rightNode = std::make_shared<TileNode>(
			BWAPI::TilePosition(currentNode->tile.x + 1, currentNode->tile.y),
			currentNode->cost + 1,
			evaluateTileNode(rightNode),
			currentNode.get()
		);
		if (isValidAndBuildable(rightNode->tile))
		{
			openList.push(rightNode);
		}

	}


}

int ScoutManager::evaluateTileNode(std::shared_ptr<TileNode> node)
{
	BWAPI::TilePosition tileLocation = node->tile;
	BWAPI::TilePosition enemyLocation = globalManager->enemyLocation;
	int distance = tileLocation.getDistance(enemyLocation);
	int maxEstimatedVal = 84659; // a random max number to compare and contrast
	return maxEstimatedVal - distance;
}

bool ScoutManager::isValidAndBuildable(BWAPI::TilePosition tile)
{
	return false;
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

void ScoutManager::checkForResources()
{
	if (!scout) // avoiding scout nullptr error when no scout is present
	{
		return;
	}
	BWAPI::Position scoutPosition = scout->getPosition();
	BWAPI::Unitset nearbyUnits = BWAPI::Broodwar->getUnitsInRadius(scoutPosition, 5000);

	ResourceList* rObj = new ResourceList();
	for (auto& unit : nearbyUnits)
	{
		rObj->location = unit->getPosition();
		// Check if the unit is a resource 
		if (unit->getType().isMineralField())
		{
			// increase the resource count
			rObj->countOfMinerals++;
		}
	}
	expansionLocations.push_back(rObj);
}

void ScoutManager::buildExpansionBase()
{
	const int expansionBaseCost = BWAPI::Broodwar->self()->getRace().getResourceDepot().mineralPrice();
	const int availableMinerals = BWAPI::Broodwar->self()->minerals();

	if (availableMinerals > expansionBaseCost)
	{
		BWAPI::Unit worker;
		bool workerFound = false;
		int resourceDepotCount = 0;
		const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
		for (auto& unit : myUnits)
		{
			if (unit->getType().isWorker() && !(unit->isConstructing()) && !workerFound && !expansionBaseBuildDispatched)
			{
				worker = unit;
				workerFound = true;
			}
			if (unit->getType().isResourceDepot())
			{
				resourceDepotCount++;
			}
		}

		if (workerFound && worker && (resourceDepotCount < 2))
		{
			bool isValidLocation = bestExpansionLocation.isValid();
			if (isValidLocation)
			{
				worker->build(BWAPI::Broodwar->self()->getRace().getResourceDepot(), BWAPI::TilePosition(bestExpansionLocation));
				expansionBaseBuildDispatched = true;
			}
		}
	}
}

void ScoutManager::calculateBestPlaceForExapnsion()
{
	double bestScore = -std::numeric_limits<double>::max(); // max negative val

	for (const auto& expansion : expansionLocations)
	{
		auto playerLocation = globalManager->playerLocation;
		auto expansionLocation = BWAPI::TilePosition(expansion->location);
		double distanceToPlayer = playerLocation.getDistance(expansionLocation);
		double distanceToEnemy = enemyLocation.getDistance(expansionLocation);
		if (distanceToPlayer < 50.00)
		{
			continue;
		}
		double score = distanceToEnemy - distanceToPlayer;
		if (score > bestScore)
		{
			bestScore = score;
			bestExpansionLocation = expansion->location;
		}
	}

	// finding a valid build location by traversing tiles nearby
	BWAPI::UnitType resourceDepotType = BWAPI::Broodwar->self()->getRace().getResourceDepot();
	int r = 10;
	for (int x = -r; x <= r; x++)
	{
		for (int y = -r; y <= r; y++)
		{
			BWAPI::TilePosition temporaryLocation(bestExpansionLocation.x + x, bestExpansionLocation.y + y);

			if (BWAPI::Broodwar->canBuildHere(temporaryLocation, resourceDepotType))
			{
				bestExpansionLocation = BWAPI::Position(temporaryLocation);
				break;
			}
		}
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
		int stepSize = map->width() / 12;
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
			if (isWalkable && (!explored && (mapVal < 400)))
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

bool isValidAndBuildable(BWAPI::TilePosition tile) {
	// Check if tile position is valid position
	if (!tile.isValid()) {
		return false;
	}

	// Check if tile position is buildable
	if (!BWAPI::Broodwar->isBuildable(tile)) {
		return false;
	}

	// Tile position is valid and buildable
	return true;
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
	checkForResources();
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
	// case if scout died before finding enemy
	else if (!enemyFound && !(scout && scout->exists()))
	{
		scoutStatus = "exploring";
		const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
		for (auto& unit : myUnits)
		{
			
			if (unit->getType().isWorker() && unit->isGatheringMinerals())
			{
				scout = unit;
				break;
			}
		}
	}

	if ((bestExpansionLocation.x != 0) && (bestExpansionLocation.y != 0))
	{
		BWAPI::Broodwar->drawCircleMap(bestExpansionLocation, 40, BWAPI::Colors::Yellow);
		BWAPI::Broodwar->drawCircleMap(bestExpansionLocation, 45, BWAPI::Colors::Yellow);
		BWAPI::Broodwar->drawCircleMap(bestExpansionLocation, 50, BWAPI::Colors::Yellow);
		BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 50), "Expansion Location Found!");
		BWAPI::TilePosition BELTilePos(bestExpansionLocation);
		std::string locationStr = "Location: " + std::to_string(BELTilePos.x) + ", " + std::to_string(BELTilePos.y);
		BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 60), locationStr.c_str());
		buildExpansionBase();
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
