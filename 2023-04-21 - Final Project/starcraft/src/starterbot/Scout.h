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
	std::map<BWAPI::Unit, BWAPI::Position>  lastKnownEnemyUnitLocations;
	std::map<BWAPI::Unit, std::string>		lastKnownEnemyUnitNames;
	void checkIfEnemyFound();
	void updateLastKnownEnemyUnitLocations();
	void drawCirclesInMiniMap();
	void detectEnemyBuildings();
	void detectEnemyUnits();
public:
	ScoutManager(MapTools * mapInstance, GlobalManager* globalManagerInstance);
	void update();
	BWAPI::Unit getScout();
	void setScout(BWAPI::Unit unit);
	BWAPI::TilePosition getEnemyLocation();
	std::string getScoutStatus();
	std::map<BWAPI::Unit, BWAPI::Position>  getLastKnownEnemyUnitLocations();
};
