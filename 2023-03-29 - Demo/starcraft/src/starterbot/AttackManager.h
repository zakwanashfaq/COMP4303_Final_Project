#pragma once
#include <BWAPI.h>
#include <set>
#include <vector>

class AttackManager
{
	bool attackEnemyBase = false;
	BWAPI::Position enemyPos;
public:
	AttackManager();
	void update();
	void attackEnemyAtBase();
	void attackAtEnemyBaseLocation();
	bool enemyDetectedAtBase();
	void setAttackEnemyStatus(bool attackBool);
};

