#include "AttackManager.h"

AttackManager::AttackManager(GlobalManager* globalManagerInstance)
{
    globalManager = globalManagerInstance;
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
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 85), "Defending base");
    auto myUnits = BWAPI::Broodwar->self()->getUnits();
    std::vector<BWAPI::Unit> attackingUnits;
    for (auto& unit : myUnits)
    {
        // If the unit is a combat unit and is not already attacking, add to attacking queue
        if (unit->getType().canAttack() && !unit->isAttacking())
        {
            attackingUnits.push_back(unit);
        }
    }

    // moving attacking units to move to the enemy base
    for (auto& unit : attackingUnits)
    {
        attackNearbyEnemyUnits(unit);
    }
}

void AttackManager::attackAtEnemyBaseLocation()
{
    auto myUnits = BWAPI::Broodwar->self()->getUnits();
    std::vector<BWAPI::Unit> attackingUnits;
    for (auto& unit : myUnits)
    {
        // If the unit is a combat unit and is not already attacking, add to attacking queue
        if (unit->getType().canAttack() && !unit->isAttacking())
        {
            attackingUnits.push_back(unit);
        }
    }

    // moving attacking units to move to the enemy base
    for (auto& unit : attackingUnits)
    {
        unit->attack(BWAPI::Position(globalManager->enemyLocation));
        attackNearbyEnemyUnits(unit);
    }
}

bool AttackManager::enemyDetectedAtBase()
{
    
    BWAPI::TilePosition my_location = BWAPI::Broodwar->self()->getStartLocation();
    BWAPI::Unit playerMainBuilding = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(my_location), BWAPI::Filter::IsResourceDepot);
    if (playerMainBuilding)
    {
        auto enemyUnits = playerMainBuilding->getUnitsInRadius(600, BWAPI::Filter::IsEnemy);
        for (auto& unit : enemyUnits)
        {
            BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 75), "Enemy at base");
            return true;
        }
    }

    return false;
}

void AttackManager::attackNearbyEnemyUnits(BWAPI::Unit unit)
{
    ;
    BWAPI::TilePosition unit_location(unit->getPosition());
    auto enemyUnits = unit->getUnitsInRadius(600, BWAPI::Filter::IsEnemy);
    for (auto& e_unit : enemyUnits)
    {
        if ((e_unit->getHitPoints() > 0) && !e_unit->getType().isBuilding())
        {
            unit->attack(e_unit);
            return;
        }
        else if ((e_unit->getHitPoints() > 0))
        {
            unit->attack(e_unit);
            return;
        }
    }
    
}

void AttackManager::setAttackEnemyStatus(bool attackBool)
{
    attackEnemyBase = attackBool;
}
