#pragma once
#include <BWAPI.h>
#include <set>
#include <vector>
#include "GlobalManager.h"
class AttackManager
{
	GlobalManager* globalManager;
	bool attackEnemyBase = false;
	BWAPI::Position enemyPos;
public:
	AttackManager(GlobalManager* globalManagerInstance);
	void update();
	void attackEnemyAtBase();
	void attackAtEnemyBaseLocation();
	bool enemyDetectedAtBase();
	void attackNearbyEnemyUnits(BWAPI::Unit unit);
	void setAttackEnemyStatus(bool attackBool);
};

