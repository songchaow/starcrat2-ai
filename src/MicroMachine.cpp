#include "Common.h"
#include "CCBot.h"
#include "JSONTools.h"
#include "Util.h"
#include "LadderInterface.h"
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include "StackWalker.h"
#include "MicroMachine.h"
#include "Communicate.h"
#include <boost/python.hpp>
#include "Python.h"
#ifdef COMPILE_TO_LIBRARY
#define LINUX_USE_SOFTWARE_RENDER 0
#ifdef SC2API
#include "sc2utils/sc2_manage_process.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_typeenums.h"
#include "sc2api/sc2_gametypes.h"
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#define getcwd _getcwd // stupid MSFT "deprecation" warning
#else
#include <unistd.h>
#endif


std::string getexepath()
{
        char buffer[255];
        char *answer = getcwd(buffer, sizeof(buffer));
        std::string s_cwd;
        if (answer)
        {
                s_cwd = answer;
        }
        return s_cwd;
}
constexpr int size_args = 10;
constexpr int size_str = 200;
int MicroMachine::Initialize(boost::python::list argv_list)
{
	/*Py_Initialize();
	int argc = 1; wchar_t** argv_w;
	argv_w = (wchar_t**)malloc(sizeof(wchar_t*)*size_args);
	for(int i=0;i<size_args;i++)
	{
            argv_w[i] = (wchar_t*)malloc(sizeof(wchar_t)*size_str);
	}
	PySys_SetArgv(1, argv_w);
	char path_str[size_str];
	char** argv = (char**)&path_str;
	wcstombs(path_str,argv_w[0],200);
	argc = 1;
	*/
	int argc = boost::python::len(argv_list);
	char** argv = (char**)malloc(sizeof(char*)*argc);
	for(int i = 0; i < argc; i++)
	{
	    argv[i] = boost::python::extract<char*>(argv_list[i]);
	}

	std::cout << argc << std::endl;
        std::cout << argv[0] <<std::endl;	
	std::cout << "Current working directory: " << getexepath() << std::endl;
	configPath = BotConfig().ConfigFileLocation;
	config = JSONTools::ReadFile(configPath);
	if (config.length() == 0)
	{
		std::cerr << "Config file could not be found, and is required for starting the bot\n";
		std::cerr << "Please read the instructions and try again\n";
		std::cin.ignore();
		exit(-1);
	}

	std::ifstream file(configPath);
	json j;
	file >> j;

	/*if (parsingFailed)
	{
		std::cerr << "Config file could not be parsed, and is required for starting the bot\n";
		std::cerr << "Please read the instructions and try again\n";
		exit(-1);
	}*/

	bool connectToLadder = false;
	std::string botRaceString;
	std::string enemyRaceString;
	std::string mapString;
	int stepSize = 1;
	sc2::Difficulty enemyDifficulty = sc2::Difficulty::Easy;
	bool PlayVsItSelf = false;
	bool PlayerOneIsHuman = false;
	bool render = false; // TODO: read from config file
	bool remote = false;
	std::string remote_addr;
	int remote_comm_port = 0;


	if (j.count("SC2API") && j["SC2API"].is_object())
	{
		const json & info = j["SC2API"];
		JSONTools::ReadBool("ConnectToLadder", info, connectToLadder);
		JSONTools::ReadString("BotRace", info, botRaceString);
		JSONTools::ReadString("EnemyRace", info, enemyRaceString);
		JSONTools::ReadString("MapFile", info, mapString);
		JSONTools::ReadInt("StepSize", info, stepSize);
		JSONTools::ReadInt("EnemyDifficulty", info, enemyDifficulty);
		JSONTools::ReadBool("PlayAsHuman", info, PlayerOneIsHuman);
		JSONTools::ReadBool("PlayVsItSelf", info, PlayVsItSelf);
		JSONTools::ReadBool("EnableRenderer", info, render);
		JSONTools::ReadBool("Remote", info, remote);
		if(remote)
		{
		    JSONTools::ReadString("RemoteAddress", info, remote_addr);
		    JSONTools::ReadInt("CommPort", info, remote_comm_port);
		}
		bot.enable_render = render;
	}
	else
	{
		std::cerr << "Config file has no 'Game Info' object, required for starting the bot\n";
		std::cerr << "Please read the instructions and try again\n";
		std::cin.ignore();
		exit(-1);
	}

#ifndef _WIN32
	const int kMapX = 1000;
	const int kMapY = 750;
	const int kMiniMapX = 300;
	const int kMiniMapY = 300;
	if (render)
	{
		sc2::RenderSettings settings(kMapX, kMapY, kMiniMapX, kMiniMapY);
		coordinator.SetRender(settings);
	}
#endif
#if defined(__linux__)
#if LINUX_USE_SOFTWARE_RENDER
	coordinator.AddCommandLine("-osmesapath /usr/lib/x86_64-linux-gnu/libOSMesa.so");
#else
	coordinator.AddCommandLine("-eglpath libEGL.so");
#endif
#endif

	if (connectToLadder)
	{
		bool loadSettings = false;
		JSONTools::ReadBool("LoadSettings", j["SC2API"], loadSettings);
		RunBot(argc, argv, &bot, sc2::Race::Terran, loadSettings);

		return 0;
	}

	// We need to load settings only when running in local
	if (!coordinator.LoadSettings(argc, argv))
	{
		std::cout << "Unable to find or parse settings." << std::endl;
		std::cin.ignore();
		return 1;
	}



	if (PlayerOneIsHuman) {
		spectatingPlayer = CreateParticipant(Util::GetRaceFromString(enemyRaceString), &human_bot);
		otherPlayer = sc2::CreateParticipant(Util::GetRaceFromString(botRaceString), &bot);
	}
	else if (PlayVsItSelf)
	{
		spectatingPlayer = sc2::CreateParticipant(Util::GetRaceFromString(botRaceString), &bot);
		otherPlayer = sc2::CreateParticipant(Util::GetRaceFromString(botRaceString), &bot2);
	}
	else
	{
		spectatingPlayer = sc2::CreateParticipant(Util::GetRaceFromString(botRaceString), &bot);
		otherPlayer = sc2::CreateComputer(Util::GetRaceFromString(enemyRaceString), enemyDifficulty);
	}



	// WARNING: Bot logic has not been thorougly tested on step sizes > 1
	//          Setting this = N means the bot's onFrame gets called once every N frames
	//          The bot may crash or do unexpected things if its logic is not called every frame
	coordinator.SetStepSize(stepSize);
	coordinator.SetRealtime(PlayerOneIsHuman);
	//coordinator.SetRealtime(true);

	coordinator.SetParticipants({
		spectatingPlayer,
		otherPlayer
		});

	// Start the game.
	if(remote)
	{
		std::cout<<"Launching SC2 from remote..."<<std::endl;
		coordinator.RemoteLaunchStarcraft(remote_addr, remote_comm_port);
	}
	else
		coordinator.LaunchStarcraft();
	coordinator.StartGame(mapString);
	auto now = std::chrono::system_clock::now();
	// Step forward the game simulation.



		/*if (bot.GetCurrentFrame() % 10 == 0)
			durations.push_back((std::chrono::system_clock::now() - now).count());
		now = std::chrono::system_clock::now();*/
	return 0;
}

boost::python::list MicroMachine::Update()
{
	coordinator.Update();
	// get game results if game ends
	bool ended = coordinator.AllGamesEnded();
	auto result = GetOurResult();
	boost::python::list ret_val;
	ret_val.append(ended);
	ret_val.append(result);
	return ret_val; // true if game ends
}

sc2::GameResult MicroMachine::GetOurResult()
{
	auto ourID = bot.Observation()->GetPlayerID();
	const std::vector<sc2::PlayerResult> &results = bot.Observation()->GetResults();
	for(auto &result : results)
	{
		if(result.player_id == ourID)
			return result.result;
	}
	return sc2::GameResult::Undecided;
}

void MicroMachine::AddRegionMoveAttack(RegionID source, RegionID target)
{
	bot.AddRegionCommand(source, target);
}

void MicroMachine::AddRegionMoveAttack(RegionMoveCommand* command)
{
	bot.AddRegionCommand(command);
}

void MicroMachine::AddProductionCommand(ProductionCommand* command)
{
	// transform to MetaType
	MetaType m_type;
	TryCreateResults* results;
	if(command->getProductType() == ProductionCommand::ProductType::Unit)
	{
		sc2::UnitTypeID m_unit_type = static_cast<CreateCommand*>(command)->unit_type;
		UnitType unit_type(m_unit_type,bot);
		m_type = MetaType(unit_type,bot);

		results = bot.Commander().Production().TryCreate(m_type,static_cast<CreateCommand*>(command)->target_pos);
	}
	else if(command->getProductType() == ProductionCommand::ProductType::Upgrade)
	{
		CCUpgrade m_upgrade_type = static_cast<UpgradeCommand*>(command)->upgrade_type;
		m_type = MetaType(m_upgrade_type,bot);
		results = bot.Commander().Production().TryCreate(m_type);
	}
	command->result = results;
}

boost::python::object MicroMachine::GetSerializedObservation()
{
	const void* m_serialized = bot.GetSerializedObservation();
	// boost::python::str obs((const char*)m_serialized,(const char*)m_serialized+bot.BUFFER_SIZE);
	//PyObject* py_buf = PyBuffer_FromReadWriteMemory(m_serialized, bot.BUFFER_SIZE);
	//PyObject* py_buf = PyMemoryView_FromMemory((char*)const_cast<void*>(m_serialized), bot.BUFFER_SIZE, PyBUF_READ);
	//boost::python::object ret_val(boost::python::handle<PyObject>(py_buf));
	boost::python::object ret_val(boost::python::handle<PyObject>(PyMemoryView_FromMemory((char*)const_cast<void*>(m_serialized), bot.BUFFER_SIZE, PyBUF_READ)));
	return ret_val;
}

#endif
#endif
