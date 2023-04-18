#pragma once
#include <BWAPI.h>
#include "Grid.hpp"
#include <unordered_map>
#include "MapTools.h"
#include "GlobalManager.h"


class ScoutManager
{
	GlobalManager*							globalManager;
	MapTools *								map;
	BWAPI::Unit								scout = nullptr;
	std::string								scoutStatus = "None";
	Grid<int> *								scoutMap;
	int										scoutCount = 0;
	int										scoutLevel = 1;
	std::unordered_map<std::string, bool>	exploringHash;
	// enemy variables
	BWAPI::Player							enemy;
	BWAPI::TilePosition						enemyLocation;
	bool									enemyFound = false;
	std::map<int, BWAPI::Position>			lastKnownEnemyUnitLocations;
	std::map<int, std::string>				lastKnownEnemyUnitNames;
	// specific task functions
	void checkIfEnemyFound();
	void updateLastKnownEnemyUnitLocations();
	void drawCirclesInMiniMap();
	void detectEnemyBuildings();
	void detectEnemyUnits();
	void retreatScout();
	void checkIfScoutIsAtBase();
	void scoutRoam();
	void traverseMap(int startX, int endX, int stepX, int startY, int endY, int stepY);
	int	getPlayerMapPositionByQuadrent();

public:
	ScoutManager(MapTools * mapInstance, GlobalManager* globalManagerInstance);
	void update();
	void setScout(BWAPI::Unit unit);
	BWAPI::Unit getScout();
	BWAPI::TilePosition getEnemyLocation();
	std::string getScoutStatus();
	std::map<int, BWAPI::Position>  getLastKnownEnemyUnitLocations();
};
