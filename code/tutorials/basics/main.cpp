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

/**
	@file

	This example application demonstrates how to create a very simple
	render window which reacts on input from Keyboard and Mouse.
*/

// OGRE
#include <OgreException.h>

// BFG libraries
#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Controller/Controller.h>
#include <Core/Path.h>
#include <Base/ShowException.h>
#include <Core/Utils.h>
#include <Model/State.h>
#include <View/View.h>

// We use Boost.Units for typesafe calculations - which are
// essentially compile time checks for formulas.
using namespace boost::units;

using BFG::s32;
using BFG::f32;

// Client applications should use event IDs higher than 10000 to avoid
// collisions with events used within the engine.
const s32 A_EXIT = 10000;

// Here comes our first state. Most of the time we use it as Owner of objects
// or as forwarder of input (Controller) events. I.e. a state could be the
// "Main Menu", a "Movie Sequence" or the 3D part of the application.
struct GameState : BFG::State
{
	GameState(GameHandle handle, EventLoop* loop) :
	State(loop)
	{
		// This part is quite important. You must connect your event callbacks.
		// If not, the event system doesn't know you're waiting for them.
		loop->connect(A_EXIT, this, &GameState::ControllerEventHandler);
	}
	
	virtual ~GameState()
	{
		infolog << "Tutorial: Destroying GameState.";
		loop()->disconnect(A_EXIT, this);
	}

	// You may update objects and other things here.
	virtual void onTick(const quantity<si::time, f32> TSLF)
	{
		// Well there's nothing to update yet. :)
		infolog << "Time since last frame: " << TSLF.value() << "ms";
	}
	
	void onExit()
	{
		// Calling this will hold the update process of this State.
		// No further events might be received after this.
		loop()->stop();
	}

	// Callback for Input. The Controller sends input directly to this
	// state, since we told him so (in `initController').
	void ControllerEventHandler(BFG::Controller_::VipEvent* e)
	{
		switch(e->getId())
		{
			// This is the event ID we specified at the top
			case A_EXIT:
			{
				onExit();
				break;
			}
		}
	}
};

// We won't display anything, so this class remains more or less empty. In this
// engine, Model and View are separated, so as you guessed this is the same as
// the GameState, but this time for render stuff.
struct ViewState : public BFG::View::State
{
public:
	ViewState(GameHandle handle, EventLoop* loop) :
	State(handle, loop),
	mControllerMyGuiAdapter(handle, loop)
	{}

	~ViewState()
	{
		infolog << "Tutorial: Destroying ViewState.";

		// The View module must be shut down manually.
		emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
	}

	virtual void pause()
	{}

	virtual void resume()
	{}
	
private:
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

// Initializing input handling here.
void initController(BFG::GameHandle stateHandle, EventLoop* loop)
{
	// The Emitter is the standard tool to send events with.
	BFG::Emitter emitter(loop);

	// At the beginning, the Controller is "empty" and must be filled with
	// states and actions. A Controller state corresponds to a Model state
	// or a View state and in fact, they must have the same handle
	// (GameHandle).
	// This part here is necessary for Action deserialization.
	BFG::Controller_::ActionMapT actions;
	actions[A_EXIT] = "A_EXIT";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(emitter.loop(), actions);

	// Actions must be configured by XML
	BFG::Path path;
	const std::string configPath = path.Expand("TutorialBasics.xml");
	const std::string stateName = "TutorialBasics";

	// The Controller must know about the size of the window for the mouse
	BFG::View::WindowAttributes wa;
	BFG::View::queryWindowAttributes(wa);
	
	// Finally, send everything to the Controller
	BFG::Controller_::StateInsertion si(configPath, stateName, stateHandle, true, wa);
	emitter.emit<BFG::Controller_::ControlEvent>
	(
		BFG::ID::CE_LOAD_STATE,
		si
	);
}

GameHandle stateHandle = BFG::generateHandle();

boost::scoped_ptr<ViewState> gViewState;
boost::scoped_ptr<GameState> gGameState;

void* createStates(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);
	
	// The different states might be seen as different viewing points of
	// one state of an application or game. Thus they always share the same
	// handle since they work closely together.
	gViewState.reset(new ViewState(stateHandle, loop));
	gGameState.reset(new GameState(stateHandle, loop));

	initController(stateHandle, loop);
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	// Our logger. Not used here, works like cout, but without the need for
	// endl and with multiple severities: dbglog, infolog, warnlog, errlog.
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/TutorialBasics.log");
	infolog << "This is our logger!";

	EventLoop loop(true);

	const std::string caption = "Tutorial 01: Basics";
	size_t controllerFrequency = 1000;
	
	// Setting up callbacks for module initialization
	boost::scoped_ptr<BFG::Base::IEntryPoint> epGame(new BFG::Base::CEntryPoint(&createStates));
	BFG::Controller_::Main controllerMain(controllerFrequency);
	BFG::View::Main viewMain(caption);

	// The order is important.
	loop.addEntryPoint(viewMain.entryPoint());
	loop.addEntryPoint(controllerMain.entryPoint());
	loop.addEntryPoint(epGame.get());

	// Now the following line will call all entry points and run the
	// application. The only way to get past this line is to call
	// loop.stop() somewhere. We do this in our GameState.
	loop.run();

	// The loop does not run anymore. Destroy the states now.
	gViewState.reset();
	gGameState.reset();
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
