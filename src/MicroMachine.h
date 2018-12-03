#pragma once
#include "Common.h"
#include "CCBot.h"
#include "Communicate.h"
#ifdef SC2API
#include "sc2utils/sc2_manage_process.h"
#include "sc2api/sc2_api.h"
#include <string>
#include <boost/python.hpp>

class Human : public sc2::Agent {
public:
        void OnGameStart() final {
                Debug()->DebugTextOut("Human");
                Debug()->SendDebug();

        }

        void OnStep()
        {
                Control()->GetObservation();
        }

};

class MicroMachine
{
public:
	sc2::Coordinator coordinator;
	std::string configPath, config;
	// The custom bot, it will control the players.
	CCBot bot;
	CCBot bot2;
	Human human_bot;
        sc2::PlayerSetup otherPlayer;
        sc2::PlayerSetup spectatingPlayer;
	int Initialize(boost::python::list argv_list);
	void Update();
	int mmain();
	// Commands to C++-end
	void AddRegionMoveAttack(RegionID source, RegionID target);
	void AddRegionMoveAttack(RegionMoveCommand* command);
	// Raw protobuf Observation class
	boost::python::object GetSerializedObservation();
};

#endif
