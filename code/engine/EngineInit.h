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

#include <boost/function.hpp>

#include <Base/LibraryMainBase.h>
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Core/Path.h>
#include <Core/Types.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>

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
	#include <View/Interface.h>
#endif

namespace BFG {

typedef boost::function<void(EventLoop&)> InitHandlerT;

namespace detail {
	
	inline void idle(EventLoop&) {
		Base::pause();
	}
	
	template <class T>
	inline bool from_string(T& t, 
	                        const std::string& s, 
	                        std::ios_base& (*f)(std::ios_base&))
	{
		std::istringstream iss(s);
		return !(iss >> f >> t).fail();
	}
}

struct Configuration
{
	Configuration(const std::string& appName) :
	appName(appName),
	logLevel(BFG::Base::Logger::SL_WARNING),
	controllerFrequency(1000),
	handler(&detail::idle)
	{
		if (appName.empty())
			throw std::logic_error
				("BFG::Configuration: You must specify an application name.");
	}
	
	// Mandatory Parameters
	const std::string appName;

	// Optional Parameters
	BFG::Base::Logger::SeverityLevel logLevel;
	std::size_t controllerFrequency;
	std::string logName;
	std::string ip;
	std::string port;
	InitHandlerT handler;
};

//! Initializes the BFG-Engine.
BFG::Base::MainContainerT engineInit(const Configuration& cfg)
{
#ifdef BFG_USE_NETWORK
	// Parse port
	BFG::u16 intPort = 0;
	if (!detail::from_string(intPort, cfg.port, std::dec))
		throw std::runtime_error("Port not a number: " + cfg.port);

	// Determine if server or client
	bool isServer = cfg.ip.empty();
	BFG::u8     intNetworkMode = (isServer?BFG_SERVER:BFG_CLIENT);
	std::string strNetworkMode = (isServer?"Server":"Client");
#else
	std::string strNetworkMode; // For logName suffix
#endif
	
	// Set logname if there is none
	std::string logName = cfg.logName;
	if (logName.empty())
		logName = cfg.appName + strNetworkMode + ".log";

	// Initialize logger
	Path p;
	BFG::Base::Logger::Init(cfg.logLevel, p.Get(ID::P_LOGS) + "/" + logName);
	
	EventLoop loop
	(
		true,
		new EventSystem::BoostThread<>("Loop"),
		new EventSystem::InterThreadCommunication()
	);

	if (!cfg.port.empty())
		dbglog << "Starting as " + strNetworkMode;

	
	BFG::Base::MainContainerT mains;
	
#ifdef BFG_USE_NETWORK
	BFG::Base::MainContainerT::value_type networkMain(new BFG::Network::Main(&loop, intNetworkMode));
	mains.push_back(networkMain);
#else
	assert(cfg.port.empty() && "You forgot to define BFG_USE_NETWORK!");
#endif

#ifdef BFG_USE_NETWORK
	if (!isServer)
#endif
	{
#ifdef BFG_USE_AUDIO
		loop.addEntryPoint(BFG::Audio::AudioInterface::getEntryPoint());
#endif

#ifdef BFG_USE_CONTROLLER
		BFG::Base::MainContainerT::value_type controllerMain(new BFG::Controller_::Main(cfg.controllerFrequency));
		mains.push_back(controllerMain);
#endif

		
#ifdef BFG_USE_PHYSICS
		BFG::Base::MainContainerT::value_type physicsMain(new BFG::Physics::Main(&loop));
		mains.push_back(physicsMain);
#endif

#ifdef BFG_USE_VIEW
		loop.addEntryPoint(View::Interface::getEntryPoint(cfg.appName));
#endif
	}

	BFG::Base::MainContainerT::iterator it = mains.begin();
	for (; it != mains.end(); ++it)
		loop.addEntryPoint((*it)->entryPoint());
	
	loop.run();
	Emitter e(&loop);

#ifdef BFG_USE_NETWORK
	if (isServer)
	{
		e.emit<Network::ControlEvent>(ID::NE_LISTEN, intPort);
	}
	else
	{
		Network::EndpointT payload = boost::make_tuple
		(
			stringToArray<128>(cfg.ip),
			stringToArray<128>(cfg.port)
		);
		e.emit<Network::ControlEvent>(ID::NE_CONNECT, payload);
	}
#endif
	
	cfg.handler(loop);
}

} // namespace BFG

#endif
