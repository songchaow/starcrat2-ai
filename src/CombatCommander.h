#pragma once

#include "Common.h"
#include "Squad.h"
#include "SquadData.h"
#include "UnitState.h"
#include "Region.h" ////new

class CCBot;

class CombatCommander
{
	std::map<std::string, Region> m_regions; ////new
	float region_width;  ////new
	float region_height; ////new

	const int FRAME_BEFORE_SIGHTING_INVALIDATED = 25;

    CCBot &         m_bot;

    SquadData       m_squadData;
    std::vector<Unit>  m_combatUnits;
	std::map<CCUnitID, UnitState> m_unitStates;
	std::map<Unit, std::pair<CCPosition, uint32_t>> m_invisibleSighting;
    bool            m_initialized;
    bool            m_attackStarted;
	int				m_currentBaseExplorationIndex;
	//float           region_width;
	//float           region_height;
	//float           x_start[10];
	//float           x_end[10];
	//float           y_start[10];
	//float           y_end[10];
	const std::string     regions_name[10] = {
		"REGION_A","REGION_B","REGION_C","REGION_D","REGION_E","REGION_F","REGION_G","REGION_H","REGION_I","REGION_J"
	};

    void            updateScoutDefenseSquad();
    void            updateDefenseSquads();
    void            updateBackupSquads();
	void            updateHarassSquads();
	void            updateAttackSquads();
    void            updateIdleSquad();
	void			checkUnitsState();
    bool            isSquadUpdateFrame();

    Unit            findClosestDefender(const Squad & defenseSquad, const CCPosition & pos, Unit & closestEnemy, std::string type);
    Unit            findWorkerToAssignToSquad(const Squad & defenseSquad, const CCPosition & pos, Unit & closestEnemy);
	bool			ShouldWorkerDefend(const Unit & woker, const Squad & defenseSquad, const CCPosition & pos, Unit & closestEnemy);

    CCPosition      getMainAttackLocation();
	CCPosition		exploreMap();

    void            updateDefenseSquadUnits(Squad & defenseSquad, size_t flyingDefendersNeeded, size_t groundDefendersNeeded, Unit & closestEnemy);
    bool            shouldWeStartAttacking();
	void            updateAllSquads();
	void            updateAllRegionsInfo(); ////new
	void			executeCommands();

public:

    CombatCommander(CCBot & bot);


    void onStart();
    void onFrame(const std::vector<Unit> & combatUnits);
	void lowPriorityCheck();
	void CombatMove(RegionID start_region, RegionID end_region);

	std::map<Unit, std::pair<CCPosition, uint32_t>> & GetInvisibleSighting();

    void drawSquadInformation();
};

