#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"

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
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();

    // Print the number of probes to the screen
    if (getCountByUnitType(BWAPI::UnitTypes::Protoss_Probe) < 8)
    {
        // Send our idle workers to mine minerals so they don't just stand there
        sendIdleWorkersToMinerals();
        // Train more workers so we can gather more income
        trainAdditionalWorkers();
    }
    else 
    {
        sendProbesToFormALine();
    }


    // Build more supply if we are going to run out soon
    buildAdditionalSupply();

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

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

void StarterBot::sendProbesToFormALine()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    BWAPI::Position nexusPos = getNexusPosition();

    // checking top, bottom, left and right to see where there is enough space 
    // for 8 probes to queue in a straight line. 
    const int WIDTH = 10, HEIGHT = 1, RADIUS = 100;
    if (isAreaBuildable(BWAPI::Position(nexusPos.x - RADIUS, nexusPos.y + RADIUS), WIDTH, HEIGHT))
    {
        nexusPos = BWAPI::Position(nexusPos.x - RADIUS, nexusPos.y + RADIUS);
    }
    else if(isAreaBuildable(BWAPI::Position(nexusPos.x - RADIUS, nexusPos.y - RADIUS), WIDTH, HEIGHT))
    {
        nexusPos = BWAPI::Position(nexusPos.x - RADIUS, nexusPos.y - RADIUS);
    }
    else if (isAreaBuildable(BWAPI::Position(nexusPos.x - RADIUS - 15, nexusPos.y), WIDTH, HEIGHT))
    {
        nexusPos = BWAPI::Position(nexusPos.x - RADIUS - 15, nexusPos.y);
    }
    else if (isAreaBuildable(BWAPI::Position(nexusPos.x + (RADIUS * 2), nexusPos.y), WIDTH, HEIGHT))
    {
        nexusPos = BWAPI::Position(nexusPos.x + (RADIUS * 2), nexusPos.y);
    } 
    else 
    {
        BWAPI::Broodwar->printf("Could not find free position near Nexus in this map. Please restart the game and try again.");
        return;
    }

    const int numProbes = getCountByUnitType(BWAPI::UnitTypes::Protoss_Probe);

    // spacing config variables
    int probeIndex = 0;
    const int spacing = 30;

    // placing the probes in a line with the above configuration
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Probe)
        {
            const BWAPI::Position newPosition(nexusPos.x + (probeIndex * spacing), nexusPos.y /* + (probeIndex * spacing)*/);
            unit->move(newPosition);
            probeIndex++;
        }
    }

    // drawing text to screen
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 30), "Zakwan Ashfaq Zian");
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 40), "201950250");
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 55), "Deep Vasantbhai Choudhary");
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 65), "201855905");
}


// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 20;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= 2) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
    }
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