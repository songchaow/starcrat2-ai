#pragma once
#include "Common.h"
#include "CCBot.h"
#include "Communicate.h"
#ifdef SC2API
#include "sc2utils/sc2_manage_process.h"
#include "sc2api/sc2_api.h"
#include <string>
#include <boost/python.hpp>
class MicroMachine
{
public:
	sc2::Coordinator coordinator;
	std::string configPath, config;
	// The custom bot, it will control the players.
	CCBot bot;
	CCBot bot2;

	int Initialize(boost::python::list argv_list);
	void Update();

	// Commands to C++-end
	void AddRegionMoveAttack(RegionID source, RegionID target);
	void AddRegionMoveAttack(RegionMoveCommand* command);
};

#endif
