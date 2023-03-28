#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"

// we use deque to store the build order for now,
//we will be fetching build order from bin folder in next iteration.
std::deque<std::pair<int, std::string>> buildOrder = { {8,"pylon"},{10,"gateway"},{11,"assimilator"},{12,"gateway"},{13,"zealot"},
    {14,"pylon"},{17,"zealot"},{17,"zealot"},{17,"pylon"},
    {18,"forge"},{19,"pylon"}, {20,"cybernetics_core"},{22,"pylon"},{23,"dragoon"},
    {23,"dragoon"},{24,"photon_cannon"} };

// we set the state of all build order to false initially
bool isBuild[16] = { false };

StarterBot::StarterBot()
{
    
}

// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();
    globalManger = new GlobalManager(&m_mapTools);
    scouthandler = new ScoutManager(&m_mapTools, globalManger);
    attackhandler = new AttackManager(globalManger);
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();
    
    scouthandler->update();
    attackhandler->update();
    // send attack after specific number of probes being built
    int num_of_probes_to_attack = 30;
    if (getCountByUnitType(BWAPI::UnitTypes::Protoss_Probe) > num_of_probes_to_attack && (scouthandler->getScoutStatus() == "enemy_found"))
    {
        BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 55), "Attacking enemy!");
        attackhandler->setAttackEnemyStatus(true);
    }
  
    // Train more workers so we can gather more income
    trainAdditionalWorkers();

    // perform build order production
    buildOrderProduction();

    // Send our idle workers to mine minerals so they don't just stand there
    sendIdleWorkersToMinerals();

    // Build more supply if we are going to run out soon
    buildAdditionalSupply();

    if (scouthandler->getScout() == nullptr)
    {
        scoutForEnemyBase();
    }

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

int StarterBot::getCountByUnitType(const BWAPI::UnitType& unitType)
{
    int sum = 0;
    for (auto unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType() == unitType && unit->isCompleted())
        {
            sum++;
        }
    }
    return sum;
}

// todo: make a more generalized version of this for unit search
BWAPI::Position StarterBot::getNexusPosition()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
        {
            return unit->getPosition();
        }
    }
    return BWAPI::Positions::None;
}

bool StarterBot::isAreaBuildable(BWAPI::Position pos, int width, int height)
{
    // todo: convert width and height to scale with tile position as well
    return isAreaBuildable(BWAPI::TilePosition(pos), width, height);
}

bool StarterBot::isAreaBuildable(BWAPI::TilePosition pos, int width, int height)
{
    for (int x = pos.x; x < (pos.x + width); x++)
    {
        for (int y = pos.y; y < (pos.y + height); y++)
        {
            if (!BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(x, y)))
            {
                return false;
            }
        }
    }
    return true;
}

void StarterBot::scoutForEnemyBase()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type then we want to send it somewhere
        if (unit->getType().isWorker())
        {
            scouthandler->setScout(unit);
            break;
        }
    }
}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    //const int workersWanted = 38;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    int count = 0;
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            if (workersOwned == 17) {
                sendWorkerToRefinery();
            }
            else {
                // Get the closest mineral to this worker unit
                BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

                // If a valid mineral was found, right click it with the unit in order to start harvesting
                if (closestMineral) { unit->rightClick(closestMineral); }
            }
        }
    }
}

void StarterBot::sendWorkerToRefinery()
{
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    int count = 0;
    for (auto& unit : myUnits)
    {
        //send 3 workers to gas refinery
        if (count < 3) {
            // Check the unit type, if it is an worker, then we want to send it somewhere
            if (unit->getType().isWorker())
            {
                // Get the closest refinery to this worker unit
                BWAPI::Unit closestRefinery = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getStaticGeysers());
                // If a valid static geyser was found, right click it with the unit in order to start collecting
                if (closestRefinery) { unit->rightClick(closestRefinery); count++; }
            }
        }

    }
}


void StarterBot::buildOrderProduction()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());

    int minerals = BWAPI::Broodwar->self()->minerals();
    int gas = BWAPI::Broodwar->self()->gas();

    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    for (size_t i = 0; i < buildOrder.size(); i++)
    {
        if (workersOwned > buildOrder[i].first && isBuild[i] == false)
        {
            if (buildOrder[i].second == "pylon")
            {
                if (minerals > 100)
                {
                    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
                    const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
                        isBuild[i] = true;
                    }

                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "assimilator")
            {
                if (minerals > 100)
                {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Assimilator);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Assimilator");
                        isBuild[i] = true;
                    }

                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "gateway")
            {
                if (minerals > 150)
                {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Gateway);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Gateway");
                        isBuild[i] = true;
                    }

                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "forge")
            {
                if (minerals > 150)
                {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Forge);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Forge");
                        isBuild[i] = true;
                    }

                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "zealot")
            {
                if (minerals > 100) {
                    int count = 0;
                    for (auto& unit : myUnits)
                    {
                        if (count < 1)
                        {
                            if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway && unit->isIdle())
                            {
                                const bool startedBuilding = unit->build(BWAPI::UnitTypes::Protoss_Zealot);
                                if (startedBuilding)
                                {
                                    isBuild[i] = true;
                                    count++;
                                }

                            }
                        }
                    }

                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "photon_cannon")
            {
                if (minerals > 150) {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Photon_Cannon);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Photon_Cannon");
                        isBuild[i] = true;
                    }

                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "cybernetics_core")
            {
                if (minerals > 150)
                {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Cybernetics_Core");
                        isBuild[i] = true;
                    }
                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "dragoon")
            {
                if (minerals > 125 && gas > 50)
                {
                    int count = 0;
                    for (auto& unit : myUnits)
                    {
                        if (count < 1)
                        {
                            if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway && unit->isIdle())
                            {
                                const bool startedBuilding = unit->build(BWAPI::UnitTypes::Protoss_Dragoon);
                                if (startedBuilding)
                                {
                                    isBuild[i] = true;
                                    count++;
                                }
                            }
                        }
                    }
                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "citadel_of_adun")
            {
                if (minerals > 150 && gas > 100)
                {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Citadel_of_Adun);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Citadel_of_Adun");
                        isBuild[i] = true;
                    }
                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "templar_archives")
            {
                if (minerals > 150 && gas > 200)
                {
                    const bool startedBuilding = Tools::BuildBuilding(BWAPI::UnitTypes::Protoss_Templar_Archives);
                    if (startedBuilding)
                    {
                        BWAPI::Broodwar->printf("Started Building Protoss_Templar_Archives");
                        isBuild[i] = true;
                    }
                }
                else
                {
                    break;
                }
            }
            if (buildOrder[i].second == "dark_templar") {
                if (minerals > 125 && gas > 100) {
                    int count = 0;
                    for (auto& unit : myUnits)
                    {
                        if (count < 1)
                        {
                            if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway && unit->isIdle())
                            {
                                const bool startedBuilding = unit->build(BWAPI::UnitTypes::Protoss_Dark_Templar);
                                if (startedBuilding)
                                {
                                    isBuild[i] = true;
                                    count++;
                                }

                            }
                        }
                    }

                }
                else
                {
                    break;
                }
            }
        }
    }

}

void StarterBot::sendWorkerToScout()
{
}


// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());

    // get the unit pointer to my depot
    const BWAPI::Unit myDepot = Tools::GetDepot();

    // if we have a valid depot unit and it's currently not training something, train a worker
    // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
    if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }

}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{


}

// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner)
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
	
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
	
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}