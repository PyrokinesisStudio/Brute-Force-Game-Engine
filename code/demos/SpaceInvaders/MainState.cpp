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


#include "MainState.h"

#include <Core/Path.h>

#include <Event/Event.h>

#include <Model/Property/SpacePlugin.h>
#include <Model/Data/GameObjectFactory.h>

#include <Controller/Controller.h>

#include <View/SkyCreation.h>

#include "SiPropertyPlugin.h"


MainState::MainState(GameHandle handle, Event::Lane& lane) :
	State(lane),
	mLane(lane),
	mPlayer(NULL_HANDLE),
	mEnvironment(new Environment),
	mInvaderGeneral(lane.createSubLane(), mEnvironment),
	mHumanGeneral(lane.createSubLane(), mEnvironment)
{
	Path p;

	std::string level = p.Get(ID::P_SCRIPTS_LEVELS) + "spaceinvaders/";
	std::string def = p.Get(ID::P_SCRIPTS_LEVELS) + "default/";

	LevelConfig lc;

	lc.mModules.push_back(def + "Object.xml");
	lc.mAdapters.push_back(def + "Adapter.xml");
	lc.mConcepts.push_back(def + "Concept.xml");
	lc.mProperties.push_back(def + "Value.xml");

	lc.mModules.push_back(level + "Object.xml");
	lc.mAdapters.push_back(level + "Adapter.xml");
	lc.mConcepts.push_back(level + "Concept.xml");
	lc.mProperties.push_back(level + "Value.xml");

	using BFG::Property::ValueId;

	PluginId spId = ValueId::ENGINE_PLUGIN_ID;
	PluginId sipId = BFG::Property::generatePluginId<PluginId>();

	boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
	boost::shared_ptr<SiPlugin> sip(new SiPlugin(sipId));

	mPluginMap.insert(sp);
	mPluginMap.insert(sip);

	boost::shared_ptr<GameObjectFactory> gof;
	gof.reset(new GameObjectFactory(mLane, lc, mPluginMap, mEnvironment, handle));

	mSector.reset(new Sector(mLane, 1, "Blah", gof));

	// Init Controller

	BFG::Controller_::ActionMapT actions;
	actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
	actions[A_SHIP_FIRE]   = "A_SHIP_FIRE";
	actions[A_QUIT]        = "A_QUIT";
	actions[A_FPS]         = "A_FPS";
	BFG::Controller_::sendActionsToController(mLane, actions);

	const std::string config_path = p.Expand("SpaceInvaders.xml");
	const std::string state_name = "SpaceInvaders";

	BFG::View::WindowAttributes wa;
	BFG::View::queryWindowAttributes(wa);

	Controller_::StateInsertion si(config_path, state_name, generateHandle(), true, wa);

	mLane.emit(ID::CE_LOAD_STATE, si);

	// ---

	View::SkyCreation sc("sky01");
	mLane.emit(ID::VE_SET_SKY, sc);

	ObjectParameter op;
	mPlayer = generateHandle();
	op.mHandle = mPlayer;
	op.mName = "The Hero's Mighty Ship";
	op.mType = "Ship";
	op.mLocation = v3(0.0f, -NEGATIVE_SHIP_Y_POSITION, OBJECT_Z_POSITION); // - 5.0f); // + SPECIAL_PACKER_MESH_OFFSET);
	fromAngleAxis(op.mLocation.orientation, -90.0f * (float) DEG2RAD, v3::UNIT_X);
	
	gof->createGameObject(op);

	mLane.connect(A_SHIP_AXIS_Y, this, &MainState::onAxisY);
	mLane.connect(A_FPS, this, &MainState::onFps);
	mLane.connect(A_SHIP_FIRE, this, &MainState::onShipFire);
	mLane.connect(A_QUIT, this, &MainState::onQuit);
}

void MainState::onAxisY(f32 factor)
{
	mLane.emit(ID::GOE_CONTROL_YAW, factor, mPlayer);
}

void MainState::onFps(bool on)
{
	mLane.emit(BFG::ID::VE_DEBUG_FPS, on);
}

void MainState::onShipFire(BFG::s32)
{
	mLane.emit(ID::GOE_FIRE_ROCKET, Event::Void(), mPlayer);
}

void MainState::onQuit(BFG::s32)
{
	mLane.emit(ID::EA_FINISH, Event::Void());
}

void MainState::onTick(const TickTimeT timeSinceLastTick)
{
	mSector->update(timeSinceLastTick);
	mInvaderGeneral.update(timeSinceLastTick);
	mHumanGeneral.update(timeSinceLastTick);
}
