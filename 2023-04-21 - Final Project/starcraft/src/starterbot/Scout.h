#pragma once
#include <BWAPI.h>
#include <queue>
#include "Grid.hpp"
#include <unordered_map>
#include "MapTools.h"
#include "GlobalManager.h"

struct ResourceList
{
	int countOfMinerals;
	BWAPI::Position location;

	ResourceList() 
		: countOfMinerals(0) 
	{
	}
};

struct TileNode {
	BWAPI::TilePosition tile;
	int cost;
	int stateEvaluationValue;
	TileNode* parent;

	TileNode(BWAPI::TilePosition tile, int cost, int stateEvaluationValue, TileNode* parent)
		: tile(tile), cost(cost), stateEvaluationValue(stateEvaluationValue), parent(parent)
	{}
};

struct CompareNode {
	bool operator()(std::shared_ptr<TileNode> a, std::shared_ptr<TileNode> b) const {
		if (a->cost > b->cost)
		{
			return true;
		}
		else if (a->cost == b->cost)
		{
			return a->stateEvaluationValue < b->stateEvaluationValue;
		}
		return false;
	}
};

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
	std::vector<ResourceList *>				expansionLocations;
	BWAPI::Position							bestExpansionLocation;
	bool									expansionBaseBuildDispatched = false;
	// specific task functions
	void checkIfEnemyFound();
	void updateLastKnownEnemyUnitLocations();
	void drawCirclesInMiniMap();
	void detectEnemyBuildings();
	void detectEnemyUnits();
	void detectChokePoint();
	void retreatScout();
	void checkIfScoutIsAtBase();
	void checkForResources();
	void buildExpansionBase();
	void calculateBestPlaceForExapnsion();
	void scoutRoam();
	void traverseMap(int startX, int endX, int stepX, int startY, int endY, int stepY);
	int	getPlayerMapPositionByQuadrent();
	int  evaluateTileNode(std::shared_ptr<TileNode> node);
	bool isValidAndBuildable(BWAPI::TilePosition tile);

public:
	ScoutManager(MapTools * mapInstance, GlobalManager* globalManagerInstance);
	void update();
	void setScout(BWAPI::Unit unit);
	BWAPI::Unit getScout();
	BWAPI::TilePosition getEnemyLocation();
	std::string getScoutStatus();
	std::map<int, BWAPI::Position>  getLastKnownEnemyUnitLocations();
};
