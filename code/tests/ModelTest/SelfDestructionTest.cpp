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

#include <boost/test/unit_test.hpp>

#include <Event/Event.h>
#include <Event/Catcher.h>
#include <ModelTest/Utils.h>

using namespace BFG;

BOOST_AUTO_TEST_SUITE(SelfDestructionSuite)

BOOST_AUTO_TEST_CASE(SelfDestructionTest)
{
	Event::Synchronizer sync;
	Event::Lane lane(sync, 100);

	Property::PluginMapT pluginMap;
	auto go = createTestGameObject(lane, pluginMap).first;
	auto module = boost::make_shared<Module>(go->getHandle());

	// Physical
	module->mPropertyConcepts.push_back("Physical");

	// Destroyable
	setValueId(module, ID::PV_Destroyed, false);
	setValueId(module, ID::PV_Respawns, false);
	setValueId(module, ID::PV_Armor, 1.0f);
	setValueId(module, ID::PV_Damage, 0.0f);
	setValueId(module, ID::PV_RespawnCountdown, 0);
	setValueId(module, ID::PV_Effect, std::string("Explosion_medium"));

	module->mPropertyConcepts.push_back("Destroyable");

	// Self Destruction
	f32 selfDestrCountDown = 3.0f;
	setValueId(module, ID::PV_SelfDestructCountdown, selfDestrCountDown);

	module->mPropertyConcepts.push_back("SelfDestruction");

	go->attachModule(module);

	// Activate it
	const auto fsd = createTestFullSyncData();
	lane.emit(ID::PE_FULL_SYNC, fsd, go->getHandle());
	sync.start();
	sync.finish();

	BOOST_REQUIRE_EQUAL(go->isActivated(), true);
	
	Event::Catcher<GameHandle> toDestroy(lane, ID::S_DESTROY_GO, NULL_HANDLE);
	
	sync.start();
	go->update(1.0f * si::second);
	go->synchronize();
	sync.finish();
	
	BOOST_CHECK_EQUAL(toDestroy.count(), 0);
	
	sync.start();
	go->update(1.0f * si::second);
	go->synchronize();
	sync.finish();
	
	BOOST_CHECK_EQUAL(toDestroy.count(), 0);
	
	sync.start();
	go->update(1.2f * si::second);
	go->synchronize();

	go->update(1.0f * si::second);
	go->synchronize();
	sync.finish();
	
	BOOST_REQUIRE_EQUAL(toDestroy.count(), 1);
	BOOST_CHECK_EQUAL(toDestroy.payloads()[0], go->getHandle());
}

BOOST_AUTO_TEST_SUITE_END()