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

/**
	@file

	This example application demonstrates how to create a very simple
	render window which reacts on input from Keyboard and Mouse.
*/

// OGRE
#include <OgreException.h>

// BFG libraries
#include <Base/Logger.h>
#include <Base/ShowException.h>
#include <Controller/Controller.h>
#include <Core/Path.h>
#include <Core/GameHandle.h>
#include <Event/Event.h>
#include <Model/State.h>
#include <View/View.h>

// We use Boost.Units for typesafe calculations - which are
// essentially compile time checks for formulas.
using namespace boost::units;

// Client applications should use event IDs higher than 10000 to avoid
// collisions with events used within the engine.
const BFG::s32 A_EXIT = 10000;

// Here comes our first state. Most of the time we use it as Owner of objects
// or as forwarder of input (Controller) events. I.e. a state could be the
// "Main Menu", a "Movie Sequence" or the 3D part of the application.
struct GameState : BFG::State
{
	GameState(GameHandle handle, BFG::Event::Lane& lane) :
	State(lane),
	mSubLane(lane.createSubLane())
	{
		// This part is quite important. You must connect your event callbacks.
		// If not, the event system doesn't know you're waiting for them.
		mSubLane->connect(A_EXIT, this, &GameState::onExit);
		
		//! \todo Simplify Controller Init
		
		// At the beginning, the Controller is "empty" and must be filled with
		// states and actions. A Controller state corresponds to a Model state
		// or a View state and in fact, they must have the same handle
		// (GameHandle).
		// This part here is necessary for Action deserialization.
		BFG::Controller_::ActionMapT actions;
		actions[A_EXIT] = "A_EXIT";
		BFG::Controller_::fillWithDefaultActions(actions);
		BFG::Controller_::sendActionsToController(lane, actions);

		// Actions must be configured by XML
		BFG::Path path;
		const std::string configPath = path.Expand("TutorialBasics.xml");
		const std::string stateName = "TutorialBasics";

		// The Controller must know about the size of the window for the mouse
		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);
		
		// Finally, send everything to the Controller
		BFG::Controller_::StateInsertion si(configPath, stateName, handle, true, wa);
		mSubLane->emit(BFG::ID::CE_LOAD_STATE, si);
	}
	
	virtual ~GameState()
	{
		infolog << "Tutorial: Destroying GameState.";
	}

	// You may update objects and other things here.
	virtual void onTick(const quantity<si::time, BFG::f32> TSLF)
	{
		// Well there's nothing to update yet. :)
		infolog << "Time since last frame: " << TSLF.value() << "ms";
	}
	
	void onExit(BFG::s32)
	{
		// Sending this will trigger deinitialization of the engine and
		// finally stop all threads.
		mSubLane->emit(BFG::ID::EA_FINISH, BFG::Event::Void());
	}
	
	BFG::Event::SubLanePtr mSubLane;
};

// We won't display anything, so this class remains more or less empty. In this
// engine, Model and View are separated, so as you guessed this is the same as
// the GameState, but this time for render stuff.
struct ViewState : public BFG::View::State
{
public:
	ViewState(GameHandle handle, BFG::Event::Lane& lane) :
	State(handle, lane),
	mControllerMyGuiAdapter(handle, lane)
	{}

	~ViewState()
	{
		infolog << "Tutorial: Destroying ViewState.";
	}

	virtual void pause()
	{}

	virtual void resume()
	{}
	
private:
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

// This is the module initialization point for the actual game.
struct Main : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	Main()
	{}

	virtual void main(BFG::Event::Lane* lane)
	{
		GameHandle handle = BFG::generateHandle();
		
		// The different states might be seen as different viewing
		// points of one state of an application or game. Thus they
		// always share the same handle since they work closely
		// together.
		mGameState.reset(new GameState(handle, *lane));
		mViewState.reset(new ViewState(handle, *lane));
	}

	boost::scoped_ptr<GameState> mGameState;
	boost::scoped_ptr<ViewState> mViewState;
};

int main() try
{
	// Our logger. Not used here, works like cout, but without the need for
	// endl and with multiple severities: dbglog, infolog, warnlog, errlog.
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/TutorialBasics.log");
	infolog << "This is our logger!";

	const std::string caption = "Tutorial 01: Basics";
	size_t controllerFrequency = 1000;
	
	// A Lane is a part of the program which handles events in a fully
	// autonomous way. Therefore, every lane gets its own thread. They all
	// register with a synchronizer which is responsible for the information
	// exchange between.
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane controllerLane(sync, controllerFrequency, "Controller");
	BFG::Event::Lane viewLane(sync, 100, "View");
	BFG::Event::Lane gameLane(sync, 100, "Game", BFG::Event::RL3);
	
	// Setting up callbacks for module initialization
	viewLane.addEntry<BFG::View::Main>(caption);
	controllerLane.addEntry<BFG::Controller_::Main>(controllerFrequency);
	gameLane.addEntry<Main>();
	
	sync.start();
	sync.finish(true);
}
catch (Ogre::Exception& e)
{
	// showException shows the exception. On Windows you'll get a MessageBox.
	BFG::showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	BFG::showException(ex.what());
}
catch (...)
{
	BFG::showException("Unknown exception");
}
