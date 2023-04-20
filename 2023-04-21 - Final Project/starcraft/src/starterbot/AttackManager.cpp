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
    std::vector<BWAPI::Unit> keepWorkersForProduction;
    int count = 0;
    for (auto& unit : myUnits)
    {
        if (unit->getType().isWorker() && unit->isGatheringMinerals() && count < 5) {
            keepWorkersForProduction.push_back(unit);
            count++;
            continue;
        }
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
    std::vector<BWAPI::Unit> keepWorkersForProduction;
    int count = 0;
    for (auto& unit : myUnits)
    {
        if (unit->getType().isWorker() && unit->isGatheringMinerals() && count < 10) {
            keepWorkersForProduction.push_back(unit);
            count++;
            continue;
        }
        // If the unit is a combat unit and is not already attacking, add to attacking queue
        if (unit->getType().canAttack() && !unit->isAttacking())
        {
            attackingUnits.push_back(unit);
        }
    }
    // moving attacking units to move to the enemy base
    for (auto& unit : attackingUnits)
    {
        unit->move(BWAPI::Position(globalManager->enemyLocation));
        if (!unit->isAttacking())
        {
            attackNearbyEnemyUnits(unit);
        }
    }
    attackEnemyBase = false;
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

//void AttackManager::attackNearbyEnemyUnits(BWAPI::Unit unit)
//{
//    
//    BWAPI::TilePosition unit_location(unit->getPosition());
//    auto enemyUnits = unit->getUnitsInRadius(600, BWAPI::Filter::IsEnemy);
//    for (auto& e_unit : enemyUnits)
//    {
//        if ((e_unit->getHitPoints() > 0) && !e_unit->getType().isBuilding())
//        {
//            unit->attack(e_unit);
//            return;
//        }
//        else if ((e_unit->getHitPoints() > 0))
//        {
//            unit->attack(e_unit);
//            return;
//        }
//    }
//
//}

void AttackManager::attackNearbyEnemyUnits(BWAPI::Unit unit)
{
    BWAPI::TilePosition unit_location(unit->getPosition());
    auto enemyUnits = unit->getUnitsInRadius(600, BWAPI::Filter::IsEnemy);

    BWAPI::Unit attackTarget = nullptr;

    for (auto& e_unit : enemyUnits)
    {
        if ((e_unit->getHitPoints() > 0) && !e_unit->getType().isBuilding())
        {
            // If a unit that can attack back is found, attack it immediately
            if (e_unit->getType().groundWeapon() != BWAPI::WeaponTypes::None || e_unit->getType().airWeapon() != BWAPI::WeaponTypes::None)
            {
                unit->attack(e_unit);
                return;
            }
            // If no attackTarget is set yet and the unit can't attack back, set it as the target
            else if (!attackTarget)
            {
                attackTarget = e_unit;
            }
        }
        // If no other attackTarget is set and a building is found, set it as the target
        else if ((e_unit->getHitPoints() > 0) && !attackTarget)
        {
            attackTarget = e_unit;
        }
    }

    // If an attack target is set, attack it
    if (attackTarget)
    {
        unit->attack(attackTarget);
    }
}


void AttackManager::setAttackEnemyStatus(bool attackBool)
{
    attackEnemyBase = attackBool;
}
