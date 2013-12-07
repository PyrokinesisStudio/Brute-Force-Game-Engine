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

#include <Base/ShowException.h>
#include <Base/Logger.h>

#include <Core/GameHandle.h>

#include <Event/Event.h>

#include "MainState.h"
#include "AudioState.h"
#include "Globals.h"

GameHandle gStateHandle = BFG::generateHandle();

static const int EVENT_FREQU = 100;

struct AudioMain : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	virtual void main(BFG::Event::Lane* lane)
	{
		mAudioState.reset(new AudioState(lane->createSubLane()));
	}

	boost::scoped_ptr<AudioState> mAudioState;
};

struct Main : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	virtual void main(BFG::Event::Lane* lane)
	{
		mGameState.reset(new MainState(gStateHandle, *lane));
	}

	boost::scoped_ptr<MainState> mGameState;
};

struct ViewMain : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	virtual void main(BFG::Event::Lane* lane)
	{
		mViewState.reset(new ViewMainState(gStateHandle, *lane));
	}

	boost::scoped_ptr<ViewMainState> mViewState;
};

#define BFG_USE_CONTROLLER
#define BFG_USE_PHYSICS
#define BFG_USE_VIEW
#define BFG_USE_AUDIO
#include <Core/EngineInit.h>

using namespace BFG;

int main( int argc, const char* argv[] ) try
{
	Init::Configuration cfg("SpaceInvaders");
	cfg.controllerFrequency = 100;
	
	//! \todo Move this somewhere else ...
	srand(time(NULL));
	
#if defined(_DEBUG) || !defined(NDEBUG)
	cfg.logLevel = Base::Logger::SL_DEBUG;
#else
	cfg.logLevel = Base::Logger::SL_INFORMATION;
#endif

	Init::engine(cfg);
	
	// Custom States
	Init::gViewLane->addEntry<ViewMain>();
	Init::gAudioLane->addEntry<AudioMain>();
	Event::Lane gameLane(Init::gSynchronizer, 300, "Game", Event::RL3);
	gameLane.addEntry<Main>();

	Init::startEngine(cfg);
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
