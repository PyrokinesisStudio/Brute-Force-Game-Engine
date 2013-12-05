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

#include "Utils.h"

using BFG::f32;
using BFG::m3x3;

BFG::PluginId enginePluginId = BFG::Property::ValueId::ENGINE_PLUGIN_ID;


// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(PhysicalTestSuite)

BOOST_AUTO_TEST_CASE (FullSyncTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	BFG::Property::PluginMapT pluginMap;
	boost::shared_ptr<BFG::GameObject> go = createTestGameObject(lane, pluginMap).first;
	BFG::Physical physical(*go, enginePluginId);
	
	BOOST_CHECK(!go->isActivated());

	const BFG::Physics::FullSyncData fsd = createTestFullSyncData();
	lane.emit(BFG::ID::PE_FULL_SYNC, fsd, go->getHandle());
	
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

BOOST_AUTO_TEST_CASE (ViewUpdateTestAfterFullSync)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	BFG::Property::PluginMapT pluginMap;
	boost::shared_ptr<BFG::GameObject> go = createTestGameObject(lane, pluginMap).first;
	BFG::Physical physical(*go, enginePluginId);
	
	BFG::Event::Catcher<v3> cPos(lane, BFG::ID::VE_UPDATE_POSITION , go->getHandle());
	BFG::Event::Catcher<qv4> cOri(lane, BFG::ID::VE_UPDATE_ORIENTATION, go->getHandle());
	
	// Can't synchronize since no PE_FULL_SYNC has been received yet.
	BOOST_CHECK_THROW(physical.synchronize(), std::runtime_error);
	
	const BFG::Physics::FullSyncData fsd = createTestFullSyncData();
	lane.emit(BFG::ID::PE_FULL_SYNC, fsd, go->getHandle());
	
	sync.start();
	sync.finish();
	
	physical.synchronize();
	
	sync.start();
	sync.finish();
	
	BOOST_CHECK_EQUAL(cPos.count(), 1);
	BOOST_CHECK_EQUAL(cOri.count(), 1);
}

BOOST_AUTO_TEST_CASE (ViewUpdateTestAfterDelta)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	BFG::Property::PluginMapT pluginMap;
	BFG::GameHandle handle = BFG::generateHandle();
	boost::shared_ptr<BFG::GameObject> go = createTestGameObject(lane, pluginMap, handle).first;
	BFG::Physical physical(*go, enginePluginId);
	
	BFG::Event::Catcher<v3> cPos(lane, BFG::ID::VE_UPDATE_POSITION , handle);
	BFG::Event::Catcher<qv4> cOri(lane, BFG::ID::VE_UPDATE_ORIENTATION, handle);
	
	// Can't synchronize since no delta updates have been received yet.
	BOOST_CHECK_THROW(physical.synchronize(), std::runtime_error);
	
	//! \todo PE_RELATIVE_VELOCITY and PE_RELATIVE_ROTATION_VELOCITY aren't
	//!       defined nor implemented in BFG::Physics.
	//!       What are they good for? Why are they defined in the Model as
	//!       PV_RelativeVelocity and PV_RelativeRotationVelocity?
	const BFG::Physics::FullSyncData fsd = createTestFullSyncData();
	lane.emit(BFG::ID::PE_POSITION, fsd.get<0>(), handle);
	lane.emit(BFG::ID::PE_ORIENTATION, fsd.get<1>(), handle);
	lane.emit(BFG::ID::PE_VELOCITY, boost::make_tuple(fsd.get<2>(), fsd.get<3>()), handle);
	//lane.emit(BFG::ID::PE_RELATIVE_VELOCITY, fsd.get<3>(), handle);
	lane.emit(BFG::ID::PE_ROTATION_VELOCITY, boost::make_tuple(fsd.get<4>(), fsd.get<5>()), handle);
	//lane.emit(BFG::ID::PE_RELATIVE_ROTATION_VELOCITY, fsd.get<5>(), handle);
	lane.emit(BFG::ID::PE_TOTAL_MASS, fsd.get<6>(), handle);
	lane.emit(BFG::ID::PE_TOTAL_INERTIA, fsd.get<7>(), handle);
	
	sync.start();
	sync.finish();
	
	// GoValues must have been set.
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_Position, enginePluginId), fsd.get<0>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::equals(go->getValue<qv4>(BFG::ID::PV_Orientation, enginePluginId), fsd.get<1>()));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_Velocity, enginePluginId), fsd.get<2>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_RelativeVelocity, enginePluginId), fsd.get<3>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_RotationVelocity, enginePluginId), fsd.get<4>(), BFG::EPSILON_F));
	BOOST_CHECK(BFG::nearEnough(go->getValue<v3>(BFG::ID::PV_RelativeRotationVelocity, enginePluginId), fsd.get<5>(), BFG::EPSILON_F));
	BOOST_CHECK_EQUAL(go->getValue<f32>(BFG::ID::PV_Mass, enginePluginId), fsd.get<6>());
	BOOST_CHECK_EQUAL(go->getValue<m3x3>(BFG::ID::PV_Inertia, enginePluginId), fsd.get<7>());
	
	physical.synchronize();
	
	sync.start();
	sync.finish();
	
	// View Events must have been sent.
	BOOST_CHECK_EQUAL(cPos.count(), 1);
	BOOST_CHECK_EQUAL(cOri.count(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
