#pragma once
#include <BWAPI.h>
#include "MapTools.h"

class GlobalManager
{
public:
	bool									attackEnemyBase = false;
	BWAPI::TilePosition						enemyLocation;
	bool									enemyFound = false;
	std::string								scoutStatus = "None";
	MapTools*								map;

	GlobalManager(MapTools* mapInstance);

};

