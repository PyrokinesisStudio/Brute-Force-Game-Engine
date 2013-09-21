/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BFG_ENGINE_INIT
#define BFG_ENGINE_INIT

#include <sstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include <Base/LibraryMainBase.h>
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Core/Path.h>
#include <Core/Types.h>
#include <Event/Event.h>

#ifdef BFG_USE_AUDIO
	#include <Audio/Interface.h>
#endif

#ifdef BFG_USE_CONTROLLER
	#include <Controller/Controller.h>
#endif

#ifdef BFG_USE_NETWORK
	#include <Network/Network.h>
#endif

#ifdef BFG_USE_PHYSICS
	#include <Physics/Physics.h>
#endif

#ifdef BFG_USE_VIEW
	#include <View/View.h>
#endif

namespace BFG {
namespace Init {

enum RunMode
{
	RM_SERVER,
	RM_CLIENT,
	RM_SINGLE
};
	
struct Configuration
{
	Configuration(const std::string& appName, RunMode rm = RM_SINGLE) :
	appName(appName),
	runMode(rm),
	logLevel(Base::Logger::SL_WARNING),
	controllerFrequency(1000)
	{
		if (appName.empty())
			throw std::logic_error
				("BFG::Configuration: You must specify an application name.");
	}
	
	// Mandatory Parameters
	const std::string appName;
	RunMode runMode;

	// Optional Parameters
	Base::Logger::SeverityLevel logLevel;
	std::size_t controllerFrequency;
	std::string logName;
	std::string ip;
	std::string port;
};

void initLogger(const std::string& fileName, Base::Logger::SeverityLevel level)
{
	Path p;
	Base::Logger::Init(level, p.Get(ID::P_LOGS) + "/" + fileName);
}


//RunMode gRunMode = RM_SINGLE;

std::string strNetworkMode(RunMode rm)
{
	switch (rm)
	{
	case RM_SERVER:
		return "Server";
	case RM_CLIENT:
		return "Client";
	}
	
	return "";
}

Event::Synchronizer gSynchronizer;

// AUDIO ################################################
#ifdef BFG_USE_AUDIO
	const u32 AUDIO_FREQ = 100;
	boost::scoped_ptr<Event::Lane> mAudioLane;
#endif

void initAudio(const Configuration& cfg) 
{
#ifdef BFG_USE_AUDIO
	gAudioLane.reset(new Event::Lane(gSynchronizer, AUDIO_FREQ, "Audio"));
	gAudioLane->addEntry<Audio::Main>();
#endif
}

// CONTROLLER ################################################
#ifdef BFG_USE_CONTROLLER
//	#include <Controller/Controller.h>
	const u32 CONTROLLER_FREQ = 100;
	boost::scoped_ptr<Event::Lane> gControllerLane;
#endif

void initController(const Configuration& cfg)
{
#ifdef BFG_USE_CONTROLLER
	gControllerLane.reset(new Event::Lane(gSynchronizer, CONTROLLER_FREQ, "Controller"));
	gControllerLane->addEntry<Controller_::Main>(cfg.controllerFrequency);
#endif
}

// PHYSICS ################################################
#ifdef BFG_USE_PHYSICS
//	#include <Physics/Physics.h>
	const u32 PHYSICS_FREQ = 100;
	boost::scoped_ptr<Event::Lane> gPhysicsLane;
#endif

void initPhysics(const Configuration& cfg)
{
#ifdef BFG_USE_PHYSICS
	gPhysicsLane.reset(new Event::Lane(gSynchronizer, PHYSICS_FREQ, "Physics", Event::RL2);
	gPhysicsLane->addEntry<Physics::Main>();
#endif
}

// VIEW ################################################
#ifdef BFG_USE_VIEW
//	#include <View/View.h>
	const u32 VIEW_FREQ = 100;
	boost::scoped_ptr<Event::Lane> gViewLane;
#endif

void initView(const Configuration& cfg)
{
#ifdef BFG_USE_VIEW
	gViewLane.reset(new Event::Lane(gSynchronizer, VIEW_FREQ, "View"));
	gViewLane->addEntry<View::Main>(cfg.appName);
#endif
}

// NETWORK ################################################
#ifdef BFG_USE_NETWORK
	const u32 NETWORK_FREQ = 100;
	boost::scoped_ptr<Event::Lane> gNetworkLane;
#endif

void initNetwork(const Configuration& cfg)
{
#ifdef BFG_USE_NETWORK

	//! \todo This is not a good method to determine server or client!
	// Determine if server or client
	bool isServer = cfg.ip.empty();
	
	gNetworkLane.reset(new Event::Lane(gSynchronizer, NETWORK_FREQ, "Network"));

	u8 intNetworkMode = (isServer?BFG_SERVER:BFG_CLIENT);
	gNetworkLane->addEntry<Network::Main>(intNetworkMode);
#else
	assert(cfg.port.empty() && "You forgot to define BFG_USE_NETWORK!");
#endif
}

void startNetwork(const Configuration& cfg)
{
#ifdef BFG_USE_NETWORK

	u16 intPort;
	
	try 
	{
		intPort = boost::lexical_cast<u16>(cfg.port);
	} 
	catch(boost::bad_lexical_cast& e)
	{
		throw std::logic_error("Error during cast 'port'. " + cfg.port + " is probably not a number! " + e.what());
	}

	if (cfg.runMode == RM_SERVER)
	{
		gNetworkLane->emit(ID::NE_LISTEN, intPort);
	}
	else
	{
		Network::EndpointT payload = boost::make_tuple
		(
			stringToArray<128>(cfg.ip),
			stringToArray<128>(cfg.port)
		);
		gNetworkLane->emit(ID::NE_CONNECT, payload);
	}

	dbglog << "Starting as " + strNetworkMode(cfg.runMode);
#endif
}

//! Initializes the BFG-Engine.
void engine(const Configuration& cfg)
{
	initNetwork(cfg);

	if (cfg.runMode != RM_SERVER)
	{
		initPhysics(cfg);
		initController(cfg);
		initView(cfg);
		initAudio(cfg);
	}

	// Initialize logger
	if (cfg.logName.empty())
		initLogger(cfg.appName + strNetworkMode(cfg.runMode) + ".log", cfg.logLevel);
	else
		initLogger(cfg.logName, cfg.logLevel);
}

void startEngine(const Configuration& cfg)
{
	gSynchronizer.start();
	startNetwork(cfg);
	gSynchronizer.finish(Event::blockUntilFinishEvent);
	Base::pause();
}

} // namespace Init
} // namespace BFG

#endif
