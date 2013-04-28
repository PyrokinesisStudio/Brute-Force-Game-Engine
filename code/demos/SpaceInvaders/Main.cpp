/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

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

#include <OgreException.h>

#include <EventSystem/Core/EventLoop.h>

#include <Core/Utils.h>
#include <Core/Path.h>
#include <Core/ShowException.h>

#include <Audio/Audio.h>
#include <Controller/Controller.h>
#include <Physics/Physics.h>
#include <View/View.h>

#include "MainState.h"
#include "AudioState.h"
#include "Globals.h"

GameHandle stateHandle = BFG::generateHandle();

boost::scoped_ptr<ViewMainState> gViewState;
boost::scoped_ptr<MainState> gGameState;
boost::scoped_ptr<AudioState> gAudioState;

void* createStates(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);

	gViewState.reset(new ViewMainState(stateHandle, loop));
	gGameState.reset(new MainState(stateHandle, loop));
	gAudioState.reset(new AudioState);

	// Init Controller
	GameHandle handle = generateHandle();

	{
		BFG::Controller_::ActionMapT actions;
		actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
		actions[A_SHIP_FIRE]   = "A_SHIP_FIRE";
		actions[A_QUIT]        = "A_QUIT";
		actions[A_FPS]         = "A_FPS";
		BFG::Controller_::sendActionsToController(loop, actions);

		Path path;
		const std::string config_path = path.Expand("SpaceInvaders.xml");
		const std::string state_name = "SpaceInvaders";

		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);

		Controller_::StateInsertion si(config_path, state_name, handle, true, wa);

		EventFactory::Create<Controller_::ControlEvent>
		(
			loop,
			ID::CE_LOAD_STATE,
			si
		);

		loop->connect(A_SHIP_AXIS_Y, gGameState.get(), &MainState::ControllerEventHandler);
		loop->connect(A_SHIP_FIRE, gGameState.get(), &MainState::ControllerEventHandler);
		loop->connect(A_QUIT, gGameState.get(), &MainState::ControllerEventHandler);
		loop->connect(A_FPS, gGameState.get(), &MainState::ControllerEventHandler);
	}
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	srand(time(NULL));

#if defined(_DEBUG) || !defined(NDEBUG)
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Si.log");
#else
	Base::Logger::Init(Base::Logger::SL_INFORMATION, "Logs/Si.log");
#endif

	EventLoop iLoop(true);

	size_t controllerFrequency = 1000;

	const std::string caption = "Engine Test 02: Space Invaders";

	// Hack: Actually, EventLoop should take ownership over an entry point
	//       pointer, but it doesn't yet. So here comes the workaround:
	boost::scoped_ptr<Base::IEntryPoint> epView(View::Interface::getEntryPoint(caption));

	BFG::Audio::Main audioMain;
	BFG::Controller_::Main controllerMain(controllerFrequency);
	BFG::Physics::Main physicsMain;
	
	iLoop.addEntryPoint(epView.get());
	iLoop.addEntryPoint(controllerMain.entryPoint());
	iLoop.addEntryPoint(physicsMain.entryPoint());
	iLoop.addEntryPoint(audioMain.entryPoint());
	iLoop.addEntryPoint(new Base::CEntryPoint(createStates));

	iLoop.run();
	
	gViewState.reset();
	gGameState.reset();
	gAudioState.reset();
}
catch (Ogre::Exception& e)
{
	showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
