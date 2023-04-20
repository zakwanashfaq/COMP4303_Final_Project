#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include <fstream>

std::deque<std::pair<int, std::string>> buildOrder;
// we set the state of all build order to false initially
bool isBuild[20] = { false };
bool isBaseAttacked = false;

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

    // call for reading build order from bin folder
    getBuildOrder();

    // Call MapTools OnStart
    m_mapTools.onStart();
    // initializes sub-modules
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

    if (attackhandler->enemyDetectedAtBase()) {
        isBaseAttacked = true;
    }
    if (isBaseAttacked && (globalManger->scoutStatus == "enemy_found") && !(attackhandler->enemyDetectedAtBase()) && readyForAttack(20)) {
        BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 55), "Attacking enemy!");
        attackhandler->setAttackEnemyStatus(true);
    }
    if ((globalManger->scoutStatus == "enemy_found") && readyForAttack(30)) {
        BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 55), "Attacking enemy!");
        attackhandler->setAttackEnemyStatus(true);
    }

    // Train more workers so we can gather more income
    trainAdditionalWorkers();

    // check if workers are send to collect gas
    sendWorkerToRefinery();

    // perform build order production
    if (buildOrder.size() > 0) {
        buildOrderProduction();
    }

    // Send our idle workers to mine minerals so they don't just stand there
    sendIdleWorkersToMinerals();

    // Build more supply if we are going to run out soon
    buildAdditionalSupply();

    // rule-based system after buildorder is completed
    ruleBasedSystem();

    if (scouthandler->getScout() == nullptr)
    {
        scoutForEnemyBase();
    }

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

void StarterBot::getBuildOrder()
{
    std::string line;
    std::deque<std::pair<int, std::string>> buildLine;
    std::ifstream input("./../bin/BuildOrder.txt");
    for (line; std::getline(input, line);) {
        int i = 0;
        char* buf = line.data();
        char* p = strtok(buf, " ");
        char* array[2];
        while (p != NULL)
        {
            array[i++] = p;
            p = strtok(NULL, " ");
        }
        buildLine.push_back({ atoi(array[0]), array[1] });
    }
    buildOrder = buildLine;

    // We print our build order 
    for (auto& order : buildLine) {
        std::cout << order.first << " " << order.second << "\n";
    }
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

bool StarterBot::readyForAttack(int Size)
{
    auto myUnits = BWAPI::Broodwar->self()->getUnits();
    BWAPI::Unitset myWorkers;
    for (auto& unit : myUnits)
    {

        if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
        {
            // we get all the workers that are at base and are idle
            myWorkers = unit->getUnitsInRadius(600, BWAPI::Filter::CanAttack && BWAPI::Filter::IsOwned && !(BWAPI::Filter::IsWorker));
        }
    }

    int NumberOfBuildCompleted = 0;
    for (int i = 0; i < buildOrder.size(); i++) {
        if (isBuild[i] == true) {
            NumberOfBuildCompleted++;
        }
    }

    if (myWorkers.size() > Size && (NumberOfBuildCompleted == buildOrder.size())) {
        return true;
    }
    return false;
}

void StarterBot::scoutForEnemyBase()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
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
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    BWAPI::Unitset myResources;
    BWAPI::Unitset myWorkers;
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
        {
            // we get all the mineral fields near our base
            myResources = unit->getUnitsInRadius(350, BWAPI::Filter::IsMineralField);
            if (myResources.size() == 0) {
                myResources = unit->getUnitsInRadius(350 * 8, BWAPI::Filter::IsMineralField);
            }
            // we get all the workers that are at base and are idle
            myWorkers = unit->getUnitsInRadius(600, BWAPI::Filter::IsWorker && BWAPI::Filter::IsIdle && BWAPI::Filter::IsOwned);
        }

    }

    // Now, we loop over each mineral patch to check if minerals are being gathered
    for (auto& resource : myResources) {

        // If not being gathered, we sent max, 2.5 workers to gaather minerals from each resource
        if (!resource->isBeingGathered())
        {
            int count = 0;
            for (auto& worker : myWorkers) {
                if (count < 2.5) {
                    worker->gather(resource);
                    count++;
                }
            }
        }
    }
}

void StarterBot::sendWorkerToRefinery()
{
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // check if assimilator is build 
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Assimilator && unit->isCompleted())
        {
            // then, check if gas is being collected by workers, if not, make a call to collect gas
            if (!(unit->isBeingGathered())) {
                sendThreeWorkersToCollectGas(unit);
            }
        }
    }
}

void StarterBot::sendThreeWorkersToCollectGas(BWAPI::Unit refinery)
{
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    BWAPI::Unitset myWorkers;
    int count = 0;

    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
        {
            // we get all the workers that are at base and are idle
            myWorkers = unit->getUnitsInRadius(512, BWAPI::Filter::IsWorker && BWAPI::Filter::IsIdle && BWAPI::Filter::IsOwned);
        }

    }
    for (auto& unit : myWorkers)
    {
        //send 3 workers to gas refinery
        if (count < 3) {
            unit->rightClick(refinery); count++;
        }
    }
}


void StarterBot::buildOrderProduction()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());

    BWAPI::Unitset myWorkers;
    std::deque<BWAPI::TilePosition> buildingLocations;
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
        {
            // we get all the workers that are at base and are idle
            buildingLocations.push_back({ unit->getTilePosition() });
            myWorkers = unit->getUnitsInRadius(512, BWAPI::Filter::IsWorker && BWAPI::Filter::IsIdle && BWAPI::Filter::IsOwned);
        }
        if ((unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) || (unit->getType() == BWAPI::UnitTypes::Protoss_Forge) || (unit->getType() == BWAPI::UnitTypes::Protoss_Assimilator) || (unit->getType() == BWAPI::UnitTypes::Protoss_Cybernetics_Core))
        {
            buildingLocations.push_back({ unit->getTilePosition() });
        }
    }

    int minerals = BWAPI::Broodwar->self()->minerals();
    int gas = BWAPI::Broodwar->self()->gas();

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
                    int count = 0;
                    for (auto& unit : myWorkers)
                    {
                        if (count < 1) {
                            for (size_t j = 0; j < buildingLocations.size(); j++)
                            {
                                const int x = buildingLocations[j].x + 3;
                                const int y = buildingLocations[j].y + 3;
                                const bool started = unit->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::TilePosition(x, y));
                                if (started) {
                                    BWAPI::Broodwar->printf("Started Building Protoss_Photon_Cannon");
                                    count++;
                                    isBuild[i] = true;
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

void StarterBot::buildCannon()
{
    if (attackhandler->enemyDetectedAtBase()) { return; }
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();

    BWAPI::Unitset myWorkers;
    int numberOfCannons = 0;
    std::deque<BWAPI::TilePosition> buildingLocations;
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)
        {
            // we get all the workers that are at base and are idle
            buildingLocations.push_back({ unit->getTilePosition() });
            myWorkers = unit->getUnitsInRadius(512, BWAPI::Filter::IsWorker && BWAPI::Filter::IsIdle && BWAPI::Filter::IsOwned);
        }
        if ((unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) || (unit->getType() == BWAPI::UnitTypes::Protoss_Pylon) || (unit->getType() == BWAPI::UnitTypes::Protoss_Forge) || (unit->getType() == BWAPI::UnitTypes::Protoss_Assimilator) || (unit->getType() == BWAPI::UnitTypes::Protoss_Cybernetics_Core))
        {
            buildingLocations.push_back({ unit->getTilePosition() });
        }
        if ((unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon)) {
            numberOfCannons++;
        }
    }
    if (numberOfCannons < 8) {
        int count = 0;
        for (auto& unit : myWorkers)
        {
            if (count < 1) {
                for (size_t j = 0; j < buildingLocations.size(); j++)
                {
                    const int x = buildingLocations[j].x + 3;
                    const int y = buildingLocations[j].y + 3;
                    const bool started = unit->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::TilePosition(x, y));
                    if (started) {
                        // BWAPI::Broodwar->printf("Started Building Protoss_Photon_Cannon");
                        count++;
                    }

                }
            }
        }
    }
}

void StarterBot::buildGateway()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    int minerals = BWAPI::Broodwar->self()->minerals();
    int numberOfGateways = 0;
    for (auto& unit : myUnits)
    {
        if ((unit->getType() == BWAPI::UnitTypes::Protoss_Gateway))
        {
            numberOfGateways++;
        }
    }
    int count = 0;
    if (numberOfGateways < 4 && minerals > 150) {
        for (auto& unit : myUnits)
        {
            if (unit->getType().isWorker() && unit->isIdle() && count < 1) {
                const bool started = unit->build(BWAPI::UnitTypes::Protoss_Gateway);
                if (started) {
                    BWAPI::Broodwar->printf("Started Building Protoss_Gateway");
                    count++;
                }
            }
        }
    }
}

void StarterBot::continueGatewayProduction()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    int minerals = BWAPI::Broodwar->self()->minerals();
    int gas = BWAPI::Broodwar->self()->gas();
    int ZealotOrDragoon = rand() % 100;

    for (auto& unit : myUnits)
    {
        if ((unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) && unit->isIdle())
        {
            if ((ZealotOrDragoon > 30) && minerals > 100) {
                const bool started = unit->build(BWAPI::UnitTypes::Protoss_Zealot);
                if (started) {
                    return;
                }
            }
            if (ZealotOrDragoon <=30 && minerals > 125 && gas > 50)
            {
                const bool started = unit->build(BWAPI::UnitTypes::Protoss_Dragoon);
                if (started) {
                    return;
                }
            }
        }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    int NumberOfBuildCompleted = 0;
    for (int i = 0; i < buildOrder.size(); i++) {
        if (isBuild[i] == true) {
            NumberOfBuildCompleted++;
        }
    }

    // If build order is completed, we can use rule based system
    if (NumberOfBuildCompleted == buildOrder.size()) {

        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();
        if (unusedSupply < 2) {
            const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

            const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
            if (startedBuilding)
            {
                BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
            }
        }
       
    }
}

// keep building units after build order is performed
void StarterBot::ruleBasedSystem()
{
    int minerals = BWAPI::Broodwar->self()->minerals();
    int gas = BWAPI::Broodwar->self()->gas();

    int NumberOfBuildCompleted = 0;

    for (int i = 0; i < buildOrder.size(); i++) {
        if (isBuild[i] == true) {
            NumberOfBuildCompleted++;
        }
    }

    // If build order is completed, we can use rule based system
    if (NumberOfBuildCompleted == buildOrder.size()) {
        continueGatewayProduction();
        int randomNumber = rand() % 2;
        //We build cannons near every building if possible
        if (randomNumber == 0) {
            buildCannon();
        }
        if (randomNumber == 1) {
            buildGateway();
        }
    }
}

// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Debug Info Enabled.\n");
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