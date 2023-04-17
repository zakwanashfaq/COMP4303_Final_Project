#include "GlobalManager.h"

GlobalManager::GlobalManager(MapTools* mapInstance)
{
	map = mapInstance;
	playerLocation = BWAPI::Broodwar->self()->getStartLocation();
}
