#pragma once

#include "MapTools.h"
#include "Scout.h"
#include "AttackManager.h"
#include "GlobalManager.h"
#include <BWAPI.h>

class StarterBot
{
    MapTools m_mapTools;
	GlobalManager* globalManger;
	ScoutManager* scouthandler;
	AttackManager* attackhandler;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
	void sendProbesToFormALine();
	void sendWorkerToScout();
    void trainAdditionalWorkers();
    void buildAdditionalSupply();
	void buildOrderProduction();
	void sendWorkerToRefinery();
    void drawDebugInformation();
	void getBuildOrder();
	void sendThreeWorkersToCollectGas(BWAPI::Unit refinery);
	void buildCannon();
	void continueGatewayProduction();

    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);
	int  getCountByUnitType(const BWAPI::UnitType& unitType);
	BWAPI::Position getNexusPosition();
	bool isAreaBuildable(BWAPI::Position pos, int width, int height);
	bool isAreaBuildable(BWAPI::TilePosition pos, int width, int height);
	void scoutForEnemyBase();
};