/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2014 Brute-Force Games GbR

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

#include <Core/Math.h>

#include <Event/Event.h>
#include <Event/Catcher.h>
#include <ModelTest/Utils.h>
#include <View/Enums.hh>
#include <View/EffectCreation.h>
#include <Audio/Enums.hh>

using namespace BFG;

BOOST_AUTO_TEST_SUITE(Destroyable)

BOOST_AUTO_TEST_CASE(DestroyableTest)
{
    Event::Synchronizer sync;
    Event::Lane lane(sync, 100);
    
	Property::PluginMapT pluginMap;
	auto enginePid = BFG::Property::ValueId::ENGINE_PLUGIN_ID;
    
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
	setValueId(module, ID::PV_SoundEffect, std::string("Explosion_medium"));
    
	BOOST_REQUIRE_NO_THROW(module->mPropertyConcepts.push_back("Destroyable"));
    BOOST_REQUIRE_NO_THROW(go->attachModule(module));
    
	// Activate it
    const auto fsd = createTestFullSyncData();
    lane.emit(ID::PE_FULL_SYNC, fsd, go->getHandle());
    sync.start();
    sync.finish();
    BOOST_REQUIRE_EQUAL(go->isActivated(), true);
    
    Event::Catcher<GameHandle> toDestroy(lane, ID::S_DESTROY_GO, NULL_HANDLE);
	Event::Catcher<View::EffectCreation> explosionView(lane, ID::VE_EFFECT, NULL_HANDLE);
	Event::Catcher<std::string> explosionSound(lane, ID::AE_SOUND_EFFECT, NULL_HANDLE);

	Property::ValueId idDamage(ID::PV_Damage, enginePid);
	f32 damage;

	Property::ValueId idDestroyed(ID::PV_Destroyed, enginePid);
	bool destroyed;

	auto expectedDamage = 0.1f;

	do
	{
		// Send contact event with penetration of 0.001.
		Physics::ModulePenetration moPe(go->getHandle(), NULL_HANDLE, 0.001f);
		lane.emit(ID::PE_CONTACT, moPe, go->getHandle());

		sync.start();
		go->update(1.0f * si::second);
		go->synchronize();
		sync.finish();

		damage = module->mValues[idDamage];
		destroyed = module->mValues[idDestroyed];

		// Destroyable has an hard coded multiplicator of 100 for damage.
		BOOST_REQUIRE(nearEnough(damage, expectedDamage, EPSILON_F));
		BOOST_REQUIRE_EQUAL(toDestroy.count(), 0);
		BOOST_REQUIRE_EQUAL(destroyed, false);

		expectedDamage += 0.1f;
	} while (expectedDamage < 1.0f);

	Physics::ModulePenetration moPe(go->getHandle(), NULL_HANDLE, 0.0011f);
	lane.emit(ID::PE_CONTACT, moPe, go->getHandle());

	sync.start();
	go->update(1.0f * si::second);
	go->synchronize();
	sync.finish();

	go->update(1.0f * si::second);
	go->synchronize();

	damage = module->mValues[idDamage];
	destroyed = module->mValues[idDestroyed];

	BOOST_REQUIRE_EQUAL(toDestroy.count(), 1);
	BOOST_REQUIRE_EQUAL(destroyed, true);
	BOOST_CHECK_EQUAL(explosionView.count(), 1);
	BOOST_CHECK_EQUAL(explosionSound.count(), 1);
}

BOOST_AUTO_TEST_CASE(RespawnTest)
{
    Event::Synchronizer sync;
    Event::Lane lane(sync, 100);
    
	Property::PluginMapT pluginMap;
	auto enginePid = BFG::Property::ValueId::ENGINE_PLUGIN_ID;
    
	auto go = createTestGameObject(lane, pluginMap).first;
    auto module = boost::make_shared<Module>(go->getHandle());
    
	// Physical
    module->mPropertyConcepts.push_back("Physical");
    
	f32 expectedRespawnTime = 10.0f;
	f32 initialDamage = 2.0f;

	// Destroyable
    setValueId(module, ID::PV_Destroyed, true);
    setValueId(module, ID::PV_Respawns, true);
    setValueId(module, ID::PV_Armor, 1.0f);
    setValueId(module, ID::PV_Damage, initialDamage);
    setValueId(module, ID::PV_RespawnCountdown, expectedRespawnTime);
    setValueId(module, ID::PV_Effect, std::string("Explosion_medium"));
	setValueId(module, ID::PV_SoundEffect, std::string("Explosion_medium"));
    
	BOOST_REQUIRE_NO_THROW(module->mPropertyConcepts.push_back("Destroyable"));
    BOOST_REQUIRE_NO_THROW(go->attachModule(module));
    
	// Activate it
    const auto fsd = createTestFullSyncData();
    lane.emit(ID::PE_FULL_SYNC, fsd, go->getHandle());
    sync.start();
    sync.finish();
    BOOST_REQUIRE_EQUAL(go->isActivated(), true);
    
//    Event::Catcher<void> magicStop(lane, ID::GOE_CONTROL_MAGIC_STOP, go->getHandle());
	Event::Catcher<v3> updatePos(lane, ID::PE_UPDATE_POSITION, module->getHandle());
	Event::Catcher<qv4> updateOri(lane, ID::PE_UPDATE_ORIENTATION, module->getHandle());
	Event::Catcher<ID::CollisionMode> collisionMode(lane, ID::PE_UPDATE_COLLISION_MODE, module->getHandle());
	Event::Catcher<bool> simulated(lane, ID::PE_UPDATE_SIMULATION_FLAG, module->getHandle());
	Event::Catcher<bool> visible(lane, ID::VE_SET_VISIBLE, module->getHandle());

	Property::ValueId idDamage(ID::PV_Damage, enginePid);
	f32 damage;

	Property::ValueId idDestroyed(ID::PV_Destroyed, enginePid);
	bool destroyed;

	for (auto respawnTime = 0; respawnTime < expectedRespawnTime; ++ respawnTime)
	{
		sync.start();
		go->update(1.0f * si::second);
		go->synchronize();
		sync.finish();

		damage = module->mValues[idDamage];
		destroyed = module->mValues[idDestroyed];

		// Destroyable has an hard coded multiplicator of 100 for damage.
		BOOST_REQUIRE(nearEnough(damage, initialDamage, EPSILON_F));
		BOOST_REQUIRE_EQUAL(destroyed, true);
		
		BOOST_REQUIRE_EQUAL(updatePos.count(), 0);
		BOOST_REQUIRE_EQUAL(updateOri.count(), 0);
		BOOST_REQUIRE_EQUAL(collisionMode.count(), 0);
		BOOST_REQUIRE_EQUAL(simulated.count(), 0);
		BOOST_REQUIRE_EQUAL(visible.count(), 0);
//		BOOST_REQUIRE_EQUAL(magicStop.count(), 0);
	}

	sync.start();
	go->update(1.1f * si::second);
	go->synchronize();
	sync.finish();

	damage = module->mValues[idDamage];
	destroyed = module->mValues[idDestroyed];

	BOOST_REQUIRE(nearEnough(damage, 0.0f, EPSILON_F));
	BOOST_REQUIRE_EQUAL(destroyed, false);

	BOOST_REQUIRE_EQUAL(updatePos.count(), 1);
	BOOST_REQUIRE_EQUAL(updateOri.count(), 1);
	BOOST_REQUIRE_EQUAL(collisionMode.count(), 1);
	BOOST_REQUIRE_EQUAL(simulated.count(), 1);
	BOOST_REQUIRE_EQUAL(visible.count(), 1);
//	BOOST_REQUIRE_EQUAL(magicStop.count(), 1);

	BOOST_REQUIRE_EQUAL(visible.payloads()[0], true);
	BOOST_REQUIRE_EQUAL(collisionMode.payloads()[0], ID::CM_Standard);
}

BOOST_AUTO_TEST_SUITE_END()