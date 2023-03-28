#include "AttackManager.h"

AttackManager::AttackManager()
{
}

void AttackManager::update()
{
    if (attackEnemyBase)
    {
        attackAtEnemyBaseLocation();
    }
    else if (enemyDetectedAtBase())
    {
        // todo: make probes attack the enemy at base
        attackEnemyAtBase();
    }
}

void AttackManager::attackEnemyAtBase()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 85), "Attacking Enemy at base");
}

void AttackManager::attackAtEnemyBaseLocation()
{
    // Get the location of the enemy base
    BWAPI::TilePosition enemyBaseLocation = BWAPI::Broodwar->enemy()->getStartLocation();

    // Select a group of your units
    auto myUnits = BWAPI::Broodwar->self()->getUnits();
    std::vector<BWAPI::Unit> attackingUnits;
    for (auto& unit : myUnits)
    {
        // If the unit is a combat unit and is not already attacking, add it to the attacking group
        if (unit->getType().canAttack() && !unit->isAttacking())
        {
            attackingUnits.push_back(unit);
        }
    }

    // Order the attacking units to attack the enemy base
    for (auto& unit : attackingUnits)
    {
        unit->attack(BWAPI::Position(enemyBaseLocation));
    }
}

bool AttackManager::enemyDetectedAtBase()
{
    
    BWAPI::TilePosition my_location = BWAPI::Broodwar->self()->getStartLocation();
    BWAPI::Unit pMain = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(my_location), BWAPI::Filter::IsResourceDepot);
    if (pMain)
    {
        auto enemyUnits = pMain->getUnitsInRadius(600, BWAPI::Filter::IsEnemy);
        for (auto& unit : enemyUnits)
        {
            std::cout << "enemy detected at base" << std::endl;
            BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 75), "Enemy at base");
            return true;
        }
    }
    

    return false;
}

void AttackManager::setAttackEnemyStatus(bool attackBool)
{
    attackEnemyBase = attackBool;
}
