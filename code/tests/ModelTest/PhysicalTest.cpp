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

#include <Event/Event.h>

#include <Core/Math.h>

#include <Event/Catcher.h>

#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Property/SpacePlugin.h>
#include <Model/Property/Concepts/Physical.h>

#include <View/Enums.hh>

#include <boost/test/unit_test.hpp>

using BFG::f32;
using BFG::m3x3;

const BFG::GameHandle handle1 = BFG::generateHandle();
const BFG::GameHandle handle2 = BFG::generateHandle();

BFG::PluginId enginePluginId = BFG::Property::ValueId::ENGINE_PLUGIN_ID;

BFG::Physics::FullSyncData createTestFullSyncData()
{
	const v3 pos(0.1, 0.2, 0.3);
	const qv4 ori(0.4, 0.5, 0.6);
	const v3 vel(0.7, 0.8, 0.9);
	const v3 relvel(1.2, 1.5, 1.8);
	const v3 rotvel(2.1, 2.4, 2.7);
	const v3 relrotvel(3.0, 3.3, 3.6);
	const f32 mass = 999.0f;
	const m3x3 inertia(9.f,8.f,7.f,6.f,5.f,4.f,3.f,2.f,1.f);
	
	BFG::Physics::FullSyncData fsd(pos, ori, vel, relvel, rotvel, relrotvel, mass, inertia);
	return fsd;
}

boost::shared_ptr<BFG::GameObject> createTestGameObject(BFG::Event::Lane& lane)
{
	BFG::Property::PluginMapT pluginMap;
	boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(enginePluginId));
	pluginMap.insert(sp);
	
	boost::shared_ptr<BFG::Environment> environment(new BFG::Environment());
	
	boost::shared_ptr<BFG::GameObject> go
	(
		new BFG::GameObject
		(
			lane,
			handle1,
			"TestObject",
			pluginMap,
			environment
		)
	);
	return go;
}

// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(PhysicalTestSuite)

BOOST_AUTO_TEST_CASE (FullSyncTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	boost::shared_ptr<BFG::GameObject> go = createTestGameObject(lane);
	BFG::Physical physical(*go, enginePluginId);
	
	BOOST_CHECK(!go->isActivated());

	const BFG::Physics::FullSyncData fsd = createTestFullSyncData();
	lane.emit(BFG::ID::PE_FULL_SYNC, fsd, handle1);
	
	sync.start();
	sync.finish();
	
	BOOST_CHECK(go->isActivated());
	
	// GameObject::getValue
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_Position, enginePluginId), fsd.get<0>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::equals(go->getValue<qv4>(BFG::ID::PV_Orientation, enginePluginId), fsd.get<1>()));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_Velocity, enginePluginId), fsd.get<2>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_RelativeVelocity, enginePluginId), fsd.get<3>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_RotationVelocity, enginePluginId), fsd.get<4>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_RelativeRotationVelocity, enginePluginId), fsd.get<5>(), BFG::EPSILON_F));
	BOOST_CHECK_EQUAL(go->getValue<f32>(BFG::ID::PV_Mass, enginePluginId), fsd.get<6>());
	BOOST_CHECK_EQUAL(go->getValue<m3x3>(BFG::ID::PV_Inertia, enginePluginId), fsd.get<7>());
}

BOOST_AUTO_TEST_CASE (ViewUpdateTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	boost::shared_ptr<BFG::GameObject> go = createTestGameObject(lane);
	BFG::Physical physical(*go, enginePluginId);
	
	BFG::Event::Catcher<v3> cPos(lane, BFG::ID::VE_UPDATE_POSITION , handle1);
	BFG::Event::Catcher<qv4> cOri(lane, BFG::ID::VE_UPDATE_ORIENTATION, handle1);
	
	// Can't synchronize since no PE_FULL_SYNC has been received yet.
	BOOST_CHECK_THROW(physical.synchronize(), std::runtime_error);
	
	const BFG::Physics::FullSyncData fsd = createTestFullSyncData();
	lane.emit(BFG::ID::PE_FULL_SYNC, fsd, handle1);
	
	sync.start();
	sync.finish();
	
	physical.synchronize();
	
	sync.start();
	sync.finish();
	
	BOOST_CHECK_EQUAL(cPos.count(), 1);
	BOOST_CHECK_EQUAL(cOri.count(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
