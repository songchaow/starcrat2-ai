#include "CombatCommander.h"
#include "Util.h"
#include "CCBot.h"
#include "stdlib.h" ////new
#include "Communicate.h"

const size_t IdlePriority = 0;
const size_t BackupPriority = 1;
const size_t HarassPriority = 2;
const size_t AttackPriority = 2;
const size_t BaseDefensePriority = 3;
const size_t ScoutDefensePriority = 4;
const size_t DropPriority = 4;

const float DefaultOrderRadius = 25;			//Order radius is the threat awareness range of units in the squad
const float MainAttackOrderRadius = 15;
const float HarassOrderRadius = 15;
const float MainAttackMaxDistance = 20;			//Distance from the center of the Main Attack Squad for a unit to be considered in it
const float MainAttackMaxRegroupDuration = 100; //Max number of frames allowed for a regroup order
const float MainAttackRegroupCooldown = 200;	//Min number of frames required to wait between regroup orders
const float MainAttackMinRetreatDuration = 50;	//Max number of frames allowed for a regroup order

CombatCommander::CombatCommander(CCBot & bot)
    : m_bot(bot)
    , m_squadData(bot)
    , m_initialized(false)
    , m_attackStarted(false)
	, m_currentBaseExplorationIndex(0)
{

}

void CombatCommander::onStart()
{
	region_width = m_bot.Map().width() / 3.0;
	region_height = m_bot.Map().height() / 3.0;
	float x_start, y_start;
	for (int i = 0; i < 9; i++) {
		x_start = (i % 3) * region_width;
		y_start = (i / 3) * region_height;
		m_regions.insert(std::pair<std::string, Region>(regions_name[i], Region(regions_name[i], region_width, region_height, x_start, y_start,m_bot)));
	}
	m_regions.insert(std::pair<std::string, Region>(regions_name[9], Region(regions_name[9], m_bot.Map().width(), m_bot.Map().height(), 0.0, 0.0, m_bot)));
	m_squadData.clearSquadData();

    // the squad that consists of units waiting for the squad to be big enough to begin the main attack
    SquadOrder idleOrder(SquadOrderTypes::Idle, CCPosition(), DefaultOrderRadius, "Prepare for battle");
    m_squadData.addSquad("Idle", Squad("Idle", idleOrder, IdlePriority, m_bot));

	// the harass attack squad that will pressure the enemy's main base workers
	SquadOrder harassOrder(SquadOrderTypes::Harass, CCPosition(0, 0), HarassOrderRadius, "Harass");
	m_squadData.addSquad("Harass", Squad("Harass", harassOrder, HarassPriority, m_bot));

    // the main attack squad that will pressure the enemy's closest base location
    SquadOrder mainAttackOrder(SquadOrderTypes::Attack, CCPosition(0, 0), MainAttackOrderRadius, "Attack");
    m_squadData.addSquad("MainAttack", Squad("MainAttack", mainAttackOrder, MainAttackMaxRegroupDuration, MainAttackRegroupCooldown, MainAttackMinRetreatDuration, MainAttackMaxDistance, AttackPriority, m_bot));

    // the backup squad that will send reinforcements to the main attack squad
    SquadOrder backupSquadOrder(SquadOrderTypes::Attack, CCPosition(0, 0), DefaultOrderRadius, "Send backups");
    m_squadData.addSquad("Backup1", Squad("Backup1", backupSquadOrder, BackupPriority, m_bot));

    // the scout defense squad will handle chasing the enemy worker scout
	// the -5 is to prevent enemy workers (during worker rush) to get outside the base defense range
    SquadOrder enemyScoutDefense(SquadOrderTypes::Defend, m_bot.GetStartLocation(), DefaultOrderRadius - 5, "Chase scout");
    m_squadData.addSquad("ScoutDefense", Squad("ScoutDefense", enemyScoutDefense, ScoutDefensePriority, m_bot));
}

bool CombatCommander::isSquadUpdateFrame()
{
    return true;
}

void CombatCommander::executeCommands()
{
	auto &commandLists = m_bot.getCombatCommandList();
	for(const CombatCommand* c : commandLists)
	{
		switch(c->getCombatType())
		{
			case CombatCommand::CombatType::RegionMove:
			{
				const RegionMoveCommand* move_command = static_cast<const RegionMoveCommand*>(c);
				CombatMove(move_command->source, move_command->target);
				break;
		        }
			default:
				;
		}
		// TODO: get result of `CombatMove`, and store it into c.
	}
	m_bot.ClearCombatCommandList();
}

void CombatCommander::onFrame(const std::vector<Unit> & combatUnits)
{
    if (!m_attackStarted)
    {
        m_attackStarted = shouldWeStartAttacking();
    }
	updateMacroUnits();
    m_combatUnits = combatUnits;
	for (auto &combatunit : m_combatUnits) {
		for (auto &macrounit : m_macroUnits) {
			if (macrounit.getID() == combatunit.getID()) {
				combatunit.startMacro();
				break;
			}
		}
	}

	auto frame = m_bot.GetGameLoop();
    if (isSquadUpdateFrame())
    {
		updateAllRegionsInfo();  ////new
        updateIdleSquad();
        updateScoutDefenseSquad();
        updateDefenseSquads();
		updateHarassSquads();
		updateAttackSquads();
        updateBackupSquads();
    }
	m_squadData.onFrame();
	// if (frame % 2500 == 0) {
	// 	CombatMove('J', 'I'); 
	// 		//CombatMove('G', 'I');
	// }
	executeCommands();
	lowPriorityCheck();
	checkUnitsState();
}

void CombatCommander::updateMacroUnits() {
	std::vector<Unit> goodUnits;
	for (auto & unit : m_macroUnits)
	{
		if (!unit.isValid()) { continue; }
		if (unit.isBeingConstructed()) { continue; }
		if (unit.getHitPoints() <= 0) { continue; }
		if (!unit.isAlive()) { continue; }
		if (!unit.isTraining()) { continue; }
		goodUnits.push_back(unit);
	}

	m_macroUnits = goodUnits;
}

void CombatCommander::updateAllRegionsInfo() { ////new	
	int region_id;
	Region *previous_region;
	//for (int i = 0; i < 10; i++) {
	//	m_regions.at(regions_name[i]).setAllUnits();
	//}
	for (auto unit : m_combatUnits) {
		previous_region = nullptr;
		/*if (unit.executeMacro() && !unit.isTraining()) {
			unit.finishMacro();
		}*/
		region_id = (int)(unit.getPosition().x / region_width)
			+ 3 * (int)(unit.getPosition().y / region_height);
		for (int i = 0;i < 9;i ++)
		{
			if (m_regions.at(regions_name[i]).containsUnit(unit))
			{
				previous_region = &m_regions.at(regions_name[i]);
			}
		}
		if (!m_regions.at(regions_name[9]).containsUnit(unit)) {
			m_regions.at(regions_name[9]).addUnit(unit);
		}
		if (previous_region == nullptr) {
			m_regions.at(regions_name[region_id]).addUnit(unit);
		}
		if (previous_region != nullptr && previous_region->getName() != m_regions.at(regions_name[region_id]).getName()) {
			previous_region->removeUnit(unit);
			m_regions.at(regions_name[region_id]).addUnit(unit);
		}
	}
}

void CombatCommander::CombatMove(RegionID start_region, RegionID end_region) { ////new
	int start_region_id = static_cast<int>(start_region);
	int end_region_id = static_cast<int>(end_region);
	srand(time(NULL));
	float target_pos_x = 0;
	float target_pos_y = 0;	
	std::vector<Unit> region_units = m_regions.at(regions_name[start_region_id]).getUnits();
	for (auto unit : region_units) {
		while (1) {
			if (end_region_id == 9) {
				target_pos_x = (rand() % int(m_bot.Map().width() * 10 - 80)) / 10.0 + 4.0;
				target_pos_y = (rand() % int(m_bot.Map().height() * 10 - 80)) / 10.0 + 4.0;
			}
			else {
				target_pos_x = (rand() % int(region_width * 10 - 80)) / 10.0 + m_regions.at(regions_name[end_region_id]).getXstart() + 4.0;
				target_pos_y = (rand() % int(region_height * 10 - 80)) / 10.0 + m_regions.at(regions_name[end_region_id]).getYstart() + 4.0;
			}
			if (m_bot.Map().isWalkable(CCPosition(target_pos_x, target_pos_y))) {
				break;
			}
		}
		Micro::SmartMove(unit.getUnitPtr(), CCPosition(target_pos_x, target_pos_y), m_bot);
		unit.startMacro();
		bool inMacro = false;
		for (auto & macrounit : m_macroUnits) {
			if (unit.getID() == macrounit.getID()) {
				inMacro = true;
				break;
			}
		}
		if (!inMacro) {
			m_macroUnits.push_back(unit);
		}
	}
}

void CombatCommander::lowPriorityCheck()
{
	auto frame = m_bot.GetGameLoop();
	if (frame % 5)
	{
		return;
	}

	std::vector<Unit> toRemove;
	for (auto sighting : m_invisibleSighting)
	{
		if (frame + FRAME_BEFORE_SIGHTING_INVALIDATED < sighting.second.second)
		{
			toRemove.push_back(sighting.first);
		}
	}
	for (auto unit : toRemove)
	{
		m_invisibleSighting.erase(unit);
	}
}

bool CombatCommander::shouldWeStartAttacking()
{
    return m_bot.Strategy().getCurrentStrategy().m_attackCondition.eval();
}

void CombatCommander::updateIdleSquad()
{
    Squad & idleSquad = m_squadData.getSquad("Idle");
    for (auto & unit : m_combatUnits)
    {
        // if it hasn't been assigned to a squad yet, put it in the low priority idle squad
        if (m_squadData.canAssignUnitToSquad(unit, idleSquad))
        {
            idleSquad.addUnit(unit);
        }
    }

	if (idleSquad.getUnits().empty())
		return;

	if (idleSquad.needsToRetreat())
	{
		SquadOrder retreatOrder(SquadOrderTypes::Retreat, getMainAttackLocation(), DefaultOrderRadius, "Retreat!!");
		idleSquad.setSquadOrder(retreatOrder);
	}
	//regroup only after retreat
	else if (idleSquad.needsToRegroup())
	{
		SquadOrder regroupOrder(SquadOrderTypes::Regroup, getMainAttackLocation(), DefaultOrderRadius, "Regroup");
		idleSquad.setSquadOrder(regroupOrder);
	}
	else
	{
		CCTilePosition nextExpansionPosition = m_bot.Bases().getNextExpansion(Players::Self);
		SquadOrder idleOrder(SquadOrderTypes::Attack, CCPosition(nextExpansionPosition.x, nextExpansionPosition.y), DefaultOrderRadius, "Prepare for battle");
		m_squadData.addSquad("Idle", Squad("Idle", idleOrder, IdlePriority, m_bot));
	}
	auto squadUnits = idleSquad.getUnits();
	bool find;
	for (auto & idleunit : squadUnits) {
		find = false;
		for (auto & macrounit : m_macroUnits) {
			if (idleunit.getID() == macrounit.getID()) {
				idleSquad.removeUnit(idleunit);
				idleSquad.addUnit(macrounit);
				find = true;
				break;
			}
		}
		if (!find) {
			Unit unit = idleunit;
			unit.finishMacro();
			idleSquad.removeUnit(idleunit);
			idleSquad.addUnit(unit);
		}
	}
}

void CombatCommander::updateBackupSquads()
{
    if (!m_attackStarted)
    {
        return;
    }

    Squad & mainAttackSquad = m_squadData.getSquad("MainAttack");

    int backupNo = 1;
    while (m_squadData.squadExists("Backup" + std::to_string(backupNo)))
    {
        Squad & backupSquad = m_squadData.getSquad("Backup" + std::to_string(backupNo));

        for (auto & unit : m_combatUnits)
        {
            BOT_ASSERT(unit.isValid(), "null unit in combat units");

            // get every unit of a lower priority and put it into the attack squad
            if (!unit.getType().isWorker()
                && !(unit.getType().isOverlord())
                && !(unit.getType().isQueen())
                && m_squadData.canAssignUnitToSquad(unit, backupSquad))
                //TODO validate that the unit is near enough the backup squad, otherwise create another one
            {
                m_squadData.assignUnitToSquad(unit, backupSquad);
            }
        }

		if (mainAttackSquad.isSuiciding())
		{
			SquadOrder retreatOrder(SquadOrderTypes::Retreat, CCPosition(0, 0), 25, "Retreat");
			backupSquad.setSquadOrder(retreatOrder);
		}
		else
		{
			SquadOrder sendBackupsOrder(SquadOrderTypes::Attack, mainAttackSquad.calcCenter(), 25, "Send backups");
			backupSquad.setSquadOrder(sendBackupsOrder);
		}

        ++backupNo;
    }
	for (int i = 1; i < backupNo; i++) {
		Squad & backupSquad = m_squadData.getSquad("Backup" + std::to_string(i));
		auto squadUnits = backupSquad.getUnits();
		bool find;
		for (auto & backupunit : squadUnits) {
			find = false;
			for (auto & macrounit : m_macroUnits) {
				if (backupunit.getID() == macrounit.getID()) {
					backupSquad.removeUnit(backupunit);
					backupSquad.addUnit(macrounit);
					find = true;
					break;
				}
			}
			if (!find) {
				Unit unit = backupunit;
				unit.finishMacro();
				backupSquad.removeUnit(backupunit);
				backupSquad.addUnit(unit);
			}
		}
	}
}

void CombatCommander::updateHarassSquads()
{
	Squad & harassSquad = m_squadData.getSquad("Harass");
	std::vector<Unit*> idleHellions;
	std::vector<Unit*> idleBanshees;
	for (auto & unit : m_combatUnits)
	{
		BOT_ASSERT(unit.isValid(), "null unit in combat units");

		// put high mobility units in the harass squad
		if ((unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_REAPER
			|| unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_HELLION
			|| unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER
			|| unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_BANSHEE
			|| unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_THOR
			|| unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_SIEGETANK)
			&& m_squadData.canAssignUnitToSquad(unit, harassSquad))
		{
			if (unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_HELLION)
				idleHellions.push_back(&unit);
			/*else if (unit.getType().getAPIUnitType() == sc2::UNIT_TYPEID::TERRAN_BANSHEE)
				idleBanshees.push_back(&unit);*/
			else
				m_squadData.assignUnitToSquad(unit, harassSquad);
		}
	}
	if(idleHellions.size() >= 10)
	{
		for (auto hellion : idleHellions)
		{
			m_squadData.assignUnitToSquad(*hellion, harassSquad);
		}
	}
	/*if (idleBanshees.size() >= 3)
	{
		for (auto banshee : idleBanshees)
		{
			m_squadData.assignUnitToSquad(*banshee, harassSquad);
		}
	}*/

	if (harassSquad.getUnits().empty())
		return;

	SquadOrder harassOrder(SquadOrderTypes::Harass, getMainAttackLocation(), HarassOrderRadius, "Harass");
	harassSquad.setSquadOrder(harassOrder);
	auto squadUnits = harassSquad.getUnits();
	bool find;
	for (auto & harassunit : squadUnits) {
		find = false;
		for (auto & macrounit : m_macroUnits) {
			if (harassunit.getID() == macrounit.getID()) {
				harassSquad.removeUnit(harassunit);
				harassSquad.addUnit(macrounit);
				find = true;
				break;
			}
		}
		if (!find) {
			Unit unit = harassunit;
			unit.finishMacro();
			harassSquad.removeUnit(harassunit);
			harassSquad.addUnit(unit);
		}
	}
}

void CombatCommander::updateAttackSquads()
{
    if (!m_attackStarted)
    {
        return;
    }

    Squad & mainAttackSquad = m_squadData.getSquad("MainAttack");

    for (auto & unit : m_combatUnits)
    {   
        BOT_ASSERT(unit.isValid(), "null unit in combat units");

        // get every unit of a lower priority and put it into the attack squad
        if (!unit.getType().isWorker()
            && !(unit.getType().isOverlord()) 
            && !(unit.getType().isQueen()) 
            && m_squadData.canAssignUnitToSquad(unit, mainAttackSquad))
        {
            m_squadData.assignUnitToSquad(unit, mainAttackSquad);
        }
    }

    if (mainAttackSquad.getUnits().empty())
        return;

    if (mainAttackSquad.needsToRetreat())
    {
        SquadOrder retreatOrder(SquadOrderTypes::Retreat, getMainAttackLocation(), DefaultOrderRadius, "Retreat!!");
        mainAttackSquad.setSquadOrder(retreatOrder);
    }
    //regroup only after retreat
    else if (mainAttackSquad.needsToRegroup())
    {
        SquadOrder regroupOrder(SquadOrderTypes::Regroup, getMainAttackLocation(), DefaultOrderRadius, "Regroup");
        mainAttackSquad.setSquadOrder(regroupOrder);
    }
    else
    {
        SquadOrder mainAttackOrder(SquadOrderTypes::Attack, getMainAttackLocation(), MainAttackOrderRadius, "Attack");
        mainAttackSquad.setSquadOrder(mainAttackOrder);
    }
	auto squadUnits = mainAttackSquad.getUnits();
	bool find;
	for (auto & mainattackunit : squadUnits) {
		find = false;
		for (auto & macrounit : m_macroUnits) {
			if (mainattackunit.getID() == macrounit.getID()) {
				mainAttackSquad.removeUnit(mainattackunit);
				mainAttackSquad.addUnit(macrounit);
				find = true;
				break;
			}
		}
		if (!find) {
			Unit unit = mainattackunit;
			unit.finishMacro();
			mainAttackSquad.removeUnit(mainattackunit);
			mainAttackSquad.addUnit(unit);
		}
	}
}

void CombatCommander::updateScoutDefenseSquad()
{
    // if the current squad has units in it then we can ignore this
    Squad & scoutDefenseSquad = m_squadData.getSquad("ScoutDefense");

    // get the region that our base is located in
    const BaseLocation * myBaseLocation = m_bot.Bases().getPlayerStartingBaseLocation(Players::Self);
    if (!myBaseLocation)
        return;

    // get all of the enemy units in this region
    std::vector<Unit> enemyUnitsInRegion;
    for (auto & unit : m_bot.UnitInfo().getUnits(Players::Enemy))
    {
        if (myBaseLocation->containsPosition(unit.getPosition()) && unit.getType().isWorker())
        {
            enemyUnitsInRegion.push_back(unit);
        }
    }

    // if there's an enemy worker in our region then assign someone to chase him
    bool assignScoutDefender = !enemyUnitsInRegion.empty();

    // if our current squad is empty and we should assign a worker, do it
    if (assignScoutDefender)
    {
		if (!scoutDefenseSquad.isEmpty())
		{
			auto & units = scoutDefenseSquad.getUnits();
			for (auto & unit : units)
			{
				if (unit.getUnitPtr()->health < unit.getUnitPtr()->health_max * m_bot.Workers().MIN_HP_PERCENTAGE_TO_FIGHT)
				{
					m_bot.Workers().finishedWithWorker(unit);
					scoutDefenseSquad.removeUnit(unit);
				}
			}
		}

		if(scoutDefenseSquad.isEmpty())
		{
			// the enemy worker that is attacking us
			Unit enemyWorkerUnit = *enemyUnitsInRegion.begin();
			BOT_ASSERT(enemyWorkerUnit.isValid(), "null enemy worker unit");

			if (enemyWorkerUnit.isValid())
			{
				Unit workerDefender = findWorkerToAssignToSquad(scoutDefenseSquad, enemyWorkerUnit.getPosition(), enemyWorkerUnit);
				if (workerDefender.isValid())
				{
					m_squadData.assignUnitToSquad(workerDefender, scoutDefenseSquad);
				}
			}
		}
    }
    // if our squad is not empty and we shouldn't have a worker chasing then take him out of the squad
    else if (!scoutDefenseSquad.isEmpty())
    {
        for (auto & unit : scoutDefenseSquad.getUnits())
        {
            BOT_ASSERT(unit.isValid(), "null unit in scoutDefenseSquad");

            unit.stop();
            if (unit.getType().isWorker())
            {
                m_bot.Workers().finishedWithWorker(unit);
            }
        }

        scoutDefenseSquad.clear();
    }
	auto squadUnits = scoutDefenseSquad.getUnits();
	bool find;
	for (auto & scoutdefenseunit : squadUnits) {
		find = false;
		for (auto & macrounit : m_macroUnits) {
			if (scoutdefenseunit.getID() == macrounit.getID()) {
				scoutDefenseSquad.removeUnit(scoutdefenseunit);
				scoutDefenseSquad.addUnit(macrounit);
				find = true;
				break;
			}
		}
		if (!find) {
			Unit unit = scoutdefenseunit;
			unit.finishMacro();
			scoutDefenseSquad.removeUnit(scoutdefenseunit);
			scoutDefenseSquad.addUnit(unit);
		}
	}
}

void CombatCommander::updateDefenseSquads()
{
	bool workerRushed = false;
    // for each of our occupied regions
    const BaseLocation * enemyBaseLocation = m_bot.Bases().getPlayerStartingBaseLocation(Players::Enemy);
    for (const BaseLocation * myBaseLocation : m_bot.Bases().getOccupiedBaseLocations(Players::Self))
    {
        // don't defend inside the enemy region, this will end badly when we are stealing gas or cannon rushing
        if (myBaseLocation == enemyBaseLocation)
        {
            continue;
        }

        CCPosition basePosition = Util::GetPosition(myBaseLocation->getDepotPosition());

        // start off assuming all enemy units in region are just workers
		int numDefendersPerEnemyUnit = 2; // 2 might be too much if we consider them workers...
		int numDefendersPerEnemyResourceDepot = 6; // This is a minimum
		int numDefendersPerEnemyCanon = 4; // This is a minimum

        // all of the enemy units in this region
        std::vector<Unit> enemyUnitsInRegion;
		float minEnemyDistance = 0;
		Unit closestEnemy;
        bool firstWorker = true;
        for (auto & unit : m_bot.UnitInfo().getUnits(Players::Enemy))
        {
            // if it's an overlord, don't worry about it for defense, we don't care what they see
            if (unit.getType().isOverlord())
            {
                continue;
            }

            if (myBaseLocation->containsPosition(unit.getPosition()))
            {
                //we can ignore the first enemy worker in our region since we assume it is a scout (handled by scout defense)
                if (unit.getType().isWorker())
                {
					if (firstWorker)
					{
						firstWorker = false;
						continue;
					}
					workerRushed = true;
                }

				float enemyDistance = Util::Dist(unit.getPosition(), basePosition);
				if(!closestEnemy.isValid() || enemyDistance < minEnemyDistance)
				{
					minEnemyDistance = enemyDistance;
					closestEnemy = unit;
				}
                enemyUnitsInRegion.push_back(unit);
            }
        }

        // calculate how many units are flying / ground units
        int numEnemyFlyingInRegion = 0;
        int numEnemyGroundInRegion = 0;
        for (auto & unit : enemyUnitsInRegion)
        {
            BOT_ASSERT(unit.isValid(), "null enemy unit in region");

            if (unit.isFlying())
            {
                numEnemyFlyingInRegion++;
            }
            else
            {
				// Canon rush dangerous
				if (unit.getType().isAttackingBuilding())
				{
					numEnemyGroundInRegion += numDefendersPerEnemyCanon;
				}
				// Hatcheries are tanky
				else if (unit.getType().isResourceDepot())
				{
					numEnemyGroundInRegion += numDefendersPerEnemyResourceDepot;
				}
                else
                {
                    numEnemyGroundInRegion++;
                }
            }
        }

        std::stringstream squadName;
        squadName << "Base Defense " << basePosition.x << " " << basePosition.y;

        // if there's nothing in this region to worry about
        if (enemyUnitsInRegion.empty())
        {
            // if a defense squad for this region exists, remove it
            if (m_squadData.squadExists(squadName.str()))
            {
                m_squadData.getSquad(squadName.str()).clear();
            }

			if (Util::IsTerran(m_bot.GetSelfRace()))
			{
				Unit base = m_bot.Buildings().getClosestResourceDepot(basePosition);
				if (base.isValid())
				{
					if (base.isFlying())
					{
						Micro::SmartAbility(base.getUnitPtr(), sc2::ABILITY_ID::LAND, basePosition, m_bot);
					}
					else if (base.getUnitPtr()->cargo_space_taken > 0)
					{
						Micro::SmartAbility(base.getUnitPtr(), sc2::ABILITY_ID::UNLOADALL, m_bot);

						//Remove builder and gas jobs.
						for (auto worker : m_bot.Workers().getWorkers())
						{
							if (m_bot.Workers().getWorkerData().getWorkerJob(worker) != WorkerJobs::Scout)
							{
								m_bot.Workers().finishedWithWorker(worker);
							}
						}
					}
				}
			}

            // and return, nothing to defend here
            continue;
        }
        else
        {
            // if we don't have a squad assigned to this region already, create one
            if (!m_squadData.squadExists(squadName.str()))
            {
				//SquadOrder defendRegion(SquadOrderTypes::Defend, basePosition, DefaultOrderRadius, "Defend Region!");
				SquadOrder defendRegion(SquadOrderTypes::Defend, basePosition, DefaultOrderRadius, "Defend Region!");
				m_squadData.addSquad(squadName.str(), Squad(squadName.str(), defendRegion, BaseDefensePriority, m_bot));
            }
        }

        // assign units to the squad
        if (m_squadData.squadExists(squadName.str()))
        {
            Squad & defenseSquad = m_squadData.getSquad(squadName.str());

            // figure out how many units we need on defense
            int flyingDefendersNeeded = numDefendersPerEnemyUnit * numEnemyFlyingInRegion;
            int groundDefensersNeeded = numDefendersPerEnemyUnit * numEnemyGroundInRegion;

            updateDefenseSquadUnits(defenseSquad, flyingDefendersNeeded, groundDefensersNeeded, closestEnemy);
        }
        else
        {
            BOT_ASSERT(false, "Squad should have existed: %s", squadName.str().c_str());
        }

		//Protect our SCVs and lift our base
		if(Util::IsTerran(m_bot.GetSelfRace()))
		{
			Unit base = m_bot.Buildings().getClosestResourceDepot(basePosition);
			if (base.isValid())
			{
				if(base.getUnitPtr()->cargo_space_taken == 0 && m_bot.Workers().getNumWorkers() > 0)
				{
					// Hide our last SCVs (should be 5, but is higher because some workers may end up dying on the way)
					if(m_bot.Workers().getNumWorkers() <= 7)
						Micro::SmartAbility(base.getUnitPtr(), sc2::ABILITY_ID::LOADALL, m_bot);
				}
				else if(!base.isFlying() && base.getUnitPtr()->health < base.getUnitPtr()->health_max * 0.5f)
					Micro::SmartAbility(base.getUnitPtr(), sc2::ABILITY_ID::LIFT, m_bot);	//TODO do not lift if we actually have no combat unit nor in production (otherwise it's BM)
			}
		}
    }

	m_bot.Strategy().setIsWorkerRushed(workerRushed);

    // for each of our defense squads, if there aren't any enemy units near the position, clear the squad
    for (const auto & kv : m_squadData.getSquads())
    {
        const Squad & squad = kv.second;
        const SquadOrder & order = squad.getSquadOrder();

        if (order.getType() != SquadOrderTypes::Defend)
        {
            continue;
        }

        bool enemyUnitInRange = false;
        for (auto & unit : m_bot.UnitInfo().getUnits(Players::Enemy))
        {
            if (Util::Dist(unit, order.getPosition()) < order.getRadius())
            {
                enemyUnitInRange = true;
                break;
            }
        }

        if (!enemyUnitInRange)
        {
            m_squadData.getSquad(squad.getName()).clear();
        }
    }
}

void CombatCommander::updateDefenseSquadUnits(Squad & defenseSquad, size_t flyingDefendersNeeded, size_t groundDefendersNeeded, Unit & closestEnemy)
{
    auto & squadUnits = defenseSquad.getUnits();

    // if there's nothing left to defend, clear the squad
    if (flyingDefendersNeeded == 0 && groundDefendersNeeded == 0)
    {
        defenseSquad.clear();
        return;
    }

    for (auto & unit : squadUnits)
    {
		// Let injured worker return mining, no need to sacrifice it
		if (unit.getType().isWorker())
		{
			if (unit.getUnitPtr()->health < unit.getUnitPtr()->health_max * m_bot.Workers().MIN_HP_PERCENTAGE_TO_FIGHT ||
				!ShouldWorkerDefend(unit, defenseSquad, defenseSquad.getSquadOrder().getPosition(), closestEnemy))
			{
				m_bot.Workers().finishedWithWorker(unit);
				defenseSquad.removeUnit(unit);
			}
		}
        else if (unit.isAlive())
        {
			bool isUseful = (flyingDefendersNeeded > 0 && unit.canAttackAir()) || (groundDefendersNeeded > 0 && unit.canAttackGround());
			if(!isUseful)
				defenseSquad.removeUnit(unit);
        }
    }

	if (flyingDefendersNeeded > 0)
	{
		Unit defenderToAdd = findClosestDefender(defenseSquad, defenseSquad.getSquadOrder().getPosition(), closestEnemy, "air");

		while(defenderToAdd.isValid())
		{
			m_squadData.assignUnitToSquad(defenderToAdd, defenseSquad);
			defenderToAdd = findClosestDefender(defenseSquad, defenseSquad.getSquadOrder().getPosition(), closestEnemy, "air");
		}
	}

	if (groundDefendersNeeded > 0)
	{
		Unit defenderToAdd = findClosestDefender(defenseSquad, defenseSquad.getSquadOrder().getPosition(), closestEnemy, "ground");

		while (defenderToAdd.isValid())
		{
			m_squadData.assignUnitToSquad(defenderToAdd, defenseSquad);
			defenderToAdd = findClosestDefender(defenseSquad, defenseSquad.getSquadOrder().getPosition(), closestEnemy, "ground");
		}
	}
	auto squadUnits_update = defenseSquad.getUnits();
	bool find;
	for (auto & defenseunit : squadUnits_update) {
		find = false;
		for (auto & macrounit : m_macroUnits) {
			if (defenseunit.getID() == macrounit.getID()) {
				defenseSquad.removeUnit(defenseunit);
				defenseSquad.addUnit(macrounit);
				find = true;
				break;
			}
		}
		if (!find) {
			Unit unit = defenseunit;
			unit.finishMacro();
			defenseSquad.removeUnit(defenseunit);
			defenseSquad.addUnit(unit);
		}
	}
}

void CombatCommander::checkUnitsState()
{
	for (auto & state : m_unitStates)
	{
		state.second.Reset();
	}

	for (auto & unit : m_bot.Commander().getValidUnits())
	{
		auto tag = unit.getTag();

		auto it = m_unitStates.find(tag);
		if (it == m_unitStates.end())
		{
			UnitState state = UnitState(unit.getHitPoints(), unit.getShields(), unit.getEnergy());
			state.Update();
			m_unitStates[tag] = state;
			continue;
		}

		UnitState & state = it->second;
		state.Update(unit.getHitPoints(), unit.getShields(), unit.getEnergy());
		

		if (state.WasAttacked())
		{
			auto threats = Util::getThreats(unit.getUnitPtr(), m_bot.GetEnemyUnits(), m_bot);
			if (threats.size() == 0 && state.HadRecentTreats())
			{
				//Invisible unit detected
				m_bot.Strategy().setEnemyHasInvisible(true);
				m_invisibleSighting[unit] = std::pair<CCPosition, uint32_t>(unit.getPosition(), m_bot.GetGameLoop());
			}
		}
		else if ((unit.canAttackAir() || unit.canAttackGround()) && m_bot.GetGameLoop() % 10==0) // skip non-attackers
		{
			auto threats = Util::getThreats(unit.getUnitPtr(), m_bot.GetEnemyUnits(), m_bot);
			state.UpdateThreat(threats.size() != 0);
		}
	}

	std::vector<CCUnitID> toRemove;
	for (auto & state : m_unitStates)
	{
		if (!state.second.WasUpdated())
		{
			//Unit died
			toRemove.push_back(state.first);
		}
	}

	//Remove dead units from the map
	for (auto & tag : toRemove)
	{
		m_unitStates.erase(tag);
	}
}

Unit CombatCommander::findClosestDefender(const Squad & defenseSquad, const CCPosition & pos, Unit & closestEnemy, std::string type)
{
    Unit closestDefender;
    float minDistance = std::numeric_limits<float>::max();

    for (auto & unit : m_combatUnits)
    {
        BOT_ASSERT(unit.isValid(), "null combat unit");

		if (type == "air" && !unit.canAttackAir())
			continue;
		if (type == "ground" && !unit.canAttackGround())
			continue;

        if (!m_squadData.canAssignUnitToSquad(unit, defenseSquad))
        {
            continue;
        }

        float dist = Util::Dist(unit, closestEnemy);
        Squad *unitSquad = m_squadData.getUnitSquad(unit);
        if (unitSquad && (unitSquad->getName() == "MainAttack" || unitSquad->getName() == "Harass") && Util::Dist(unit.getPosition(), unitSquad->getSquadOrder().getPosition()) < dist)
        {
            //We do not want to bring back the main attackers when they are closer to their objective than our base
            continue;
        }

        if (!closestDefender.isValid() || dist < minDistance)
        {
            closestDefender = unit;
            minDistance = dist;
        }
    }

    if (!closestDefender.isValid() && type == "ground")
    {
        // we search for worker to defend.
        closestDefender = findWorkerToAssignToSquad(defenseSquad, pos, closestEnemy);
    }

    return closestDefender;
}

Unit CombatCommander::findWorkerToAssignToSquad(const Squad & defenseSquad, const CCPosition & pos, Unit & closestEnemy)
{
    // get our worker unit that is mining that is closest to it
    Unit workerDefender = m_bot.Workers().getClosestMineralWorkerTo(closestEnemy.getPosition(), m_bot.Workers().MIN_HP_PERCENTAGE_TO_FIGHT);

	if(ShouldWorkerDefend(workerDefender, defenseSquad, pos, closestEnemy))
	{
        m_bot.Workers().setCombatWorker(workerDefender);
    }
    else
    {
        workerDefender = {};
    }
    return workerDefender;
}

bool CombatCommander::ShouldWorkerDefend(const Unit & woker, const Squad & defenseSquad, const CCPosition & pos, Unit & closestEnemy)
{
	// grab it from the worker manager and put it in the squad, but only if it meets the distance requirements
	if (woker.isValid() && 
		m_squadData.canAssignUnitToSquad(woker, defenseSquad) &&
		!closestEnemy.isFlying() &&
		Util::Dist(woker, pos) < 15.f &&	// worker should not get too far from base
		(Util::Dist(woker, closestEnemy) < 7.f ||	// worker can fight only units close to it
		(closestEnemy.getType().isBuilding() && Util::Dist(closestEnemy, pos) < 12.f)))	// worker can fight buildings somewhat close to the base
		return true;
	return false;
}

std::map<Unit, std::pair<CCPosition, uint32_t>> & CombatCommander::GetInvisibleSighting()
{
	return m_invisibleSighting;
}

void CombatCommander::drawSquadInformation()
{
    m_squadData.drawSquadInformation();
}

CCPosition CombatCommander::getMainAttackLocation()
{
    const BaseLocation * enemyBaseLocation = m_bot.Bases().getPlayerStartingBaseLocation(Players::Enemy);

    // First choice: Attack an enemy region if we can see units inside it
    if (enemyBaseLocation)
    {
        CCPosition enemyBasePosition = enemyBaseLocation->getPosition();
        // If the enemy base hasn't been seen yet, go there.
        if (!m_bot.Map().isExplored(enemyBasePosition))
        {
			if (m_bot.GetCurrentFrame() % 25 == 0)
				std::cout << m_bot.GetCurrentFrame() << ": Unexplored enemy base" << std::endl;
            return enemyBasePosition;
        }
        else
        {
            // if it has been explored, go there if there are any visible enemy units there
            for (auto & enemyUnit : m_bot.UnitInfo().getUnits(Players::Enemy))
            {
                if (enemyUnit.getType().isBuilding() && Util::Dist(enemyUnit, enemyBasePosition) < 15)
                {
					if (m_bot.GetCurrentFrame() % 25 == 0)
						std::cout << m_bot.GetCurrentFrame() << ": Visible enemy building" << std::endl;
                    return enemyBasePosition;
                }
            }
        }
    }

	if(!m_bot.Strategy().shouldFocusBuildings())
	{
		m_bot.Actions()->SendChat("Looks like you lost your main base, time to conceed? :)");
		m_bot.Actions()->SendChat("Entering Derp mode... sorry for the glitches to come");
		m_bot.Strategy().setFocusBuildings(true);
	}

	CCPosition harassSquadCenter = m_squadData.getSquad("Harass").calcCenter();
	float lowestDistance = -1.f;
	CCPosition closestEnemyPosition;

    // Second choice: Attack known enemy buildings
	Squad& harassSquad = m_squadData.getSquad("Harass");
    for (const auto & enemyUnit : harassSquad.getTargets())
    {
        if (enemyUnit.getType().isBuilding() && enemyUnit.isAlive() && enemyUnit.getUnitPtr()->display_type != sc2::Unit::Hidden)
        {
			if (enemyUnit.getType().isCreepTumor())
				continue;
			float dist = Util::Dist(enemyUnit, harassSquadCenter);
			if(lowestDistance < 0 || dist < lowestDistance)
			{
				lowestDistance = dist;
				closestEnemyPosition = enemyUnit.getPosition();
			}
        }
    }
	if (lowestDistance >= 0.f)
	{
		if (m_bot.GetCurrentFrame() % 25 == 0)
			std::cout << m_bot.GetCurrentFrame() << ": Memory enemy building" << std::endl;
		return closestEnemyPosition;
	}

    // Third choice: Attack visible enemy units that aren't overlords
	for (const auto & enemyUnit : harassSquad.getTargets())
	{
        if (!enemyUnit.getType().isOverlord() && enemyUnit.isAlive() && enemyUnit.getUnitPtr()->display_type != sc2::Unit::Hidden)
        {
			if (enemyUnit.getType().isCreepTumor())
				continue;
			float dist = Util::Dist(enemyUnit, harassSquadCenter);
			if (lowestDistance < 0 || dist < lowestDistance)
			{
				lowestDistance = dist;
				closestEnemyPosition = enemyUnit.getPosition();
			}
        }
    }
	if (lowestDistance >= 0.f)
	{
		if (m_bot.GetCurrentFrame() % 25 == 0)
			std::cout << m_bot.GetCurrentFrame() << ": Memory enemy unit" << std::endl;
		return closestEnemyPosition;
	}

    // Fourth choice: We can't see anything so explore the map attacking along the way
	return exploreMap();
}

CCPosition CombatCommander::exploreMap()
{
	CCPosition basePosition = Util::GetPosition(m_bot.Bases().getBasePosition(Players::Enemy, m_currentBaseExplorationIndex));
	for (auto & unit : m_combatUnits)
	{
		if (Util::Dist(unit.getPosition(), basePosition) < 3.f)
		{
			m_currentBaseExplorationIndex = (m_currentBaseExplorationIndex + 1) % m_bot.Bases().getBaseLocations().size();
			if (m_bot.GetCurrentFrame() % 25 == 0)
				std::cout << m_bot.GetCurrentFrame() << ": Explore map, base index increased to " << m_currentBaseExplorationIndex << std::endl;
			return Util::GetPosition(m_bot.Bases().getBasePosition(Players::Enemy, m_currentBaseExplorationIndex));
		}
	}
	if (m_bot.GetCurrentFrame() % 25 == 0)
		std::cout << m_bot.GetCurrentFrame() << ": Explore map, base index " << m_currentBaseExplorationIndex << std::endl;
	return basePosition;
}
