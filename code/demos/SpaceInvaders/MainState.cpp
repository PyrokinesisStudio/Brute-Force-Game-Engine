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

#include <Model/Property/SpacePlugin.h>
#include <Model/Data/GameObjectFactory.h>
#include <Physics/Event.h>

#include "SiPropertyPlugin.h"


MainState::MainState(GameHandle handle, EventLoop* loop) :
State(loop),
mPlayer(NULL_HANDLE),
mEnvironment(new Environment),
mInvaderGeneral(loop, mEnvironment),
mHumanGeneral(loop, mEnvironment)
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
	gof.reset(new GameObjectFactory(loop, lc, mPluginMap, mEnvironment, handle));

	mSector.reset(new Sector(loop, 1, "Blah", gof));

	View::SkyCreation sc("sky01");
	emit<View::Event>(ID::VE_SET_SKY, sc);

	ObjectParameter op;
	op.mHandle = generateHandle();
	op.mName = "The Hero's Mighty Ship";
	op.mType = "Ship";
	op.mLocation = v3(0.0f, -NEGATIVE_SHIP_Y_POSITION, OBJECT_Z_POSITION); // - 5.0f); // + SPECIAL_PACKER_MESH_OFFSET);
	fromAngleAxis(op.mLocation.orientation, -90 * DEG2RAD, v3::UNIT_X);
	boost::shared_ptr<GameObject> playerShip = gof->createGameObject(op);
	mSector->addObject(playerShip);

	mPlayer = playerShip->getHandle();
}

void MainState::ControllerEventHandler(Controller_::VipEvent* e)
{
	switch(e->getId())
	{
		case A_SHIP_AXIS_Y:
			emit<GameObjectEvent>(ID::GOE_CONTROL_YAW, boost::get<float>(e->getData()), mPlayer);
			break;

		case A_SHIP_FIRE:
			emit<GameObjectEvent>(ID::GOE_FIRE_ROCKET, 0, mPlayer);
			break;

		case A_QUIT:
			loop()->stop();
			break;

		case A_FPS:
			emit<BFG::View::Event>(BFG::ID::VE_DEBUG_FPS, boost::get<bool>(e->getData()));
			break;
	}
}

void MainState::onTick(const quantity<si::time, f32> TSLF)
{
	mSector->update(TSLF);
	mInvaderGeneral.update(TSLF);
	mHumanGeneral.update(TSLF);

	emit<Physics::Event>(ID::PE_STEP, TSLF.value());
}
