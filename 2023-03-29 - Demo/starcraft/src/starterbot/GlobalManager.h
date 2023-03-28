#pragma once
#include <BWAPI.h>


class GlobalManager
{
	bool attackEnemyBase = false;
	BWAPI::Position enemyPos;
	BWAPI::TilePosition						enemyLocation;
	bool									enemyFound = false;
	std::string								scoutStatus = "None";

public:
	GlobalManager();

};

