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

#include <Core/Path.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Property/SpacePlugin.h>
#include <Model/Data/GameObjectFactory.h>
#include <Model/Sector.h>

#include "Utils.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(SectorTestSuite)

BOOST_AUTO_TEST_CASE (CreateObjectTest)
{
	using BFG::Property::ValueId;
	using namespace BFG;

	// Event System setup
	Event::Synchronizer sync;
	Event::Lane lane(sync, 100);
	
	// XML setup
	Path p;
	std::string def = p.Get(ID::P_SCRIPTS_LEVELS) + "default/";

	LevelConfig lc;
	lc.mModules.push_back(def + "Object.xml");
	lc.mAdapters.push_back(def + "Adapter.xml");
	lc.mConcepts.push_back(def + "Concept.xml");
	lc.mProperties.push_back(def + "Value.xml");

	// Plugin setup
	PluginId spId = ValueId::ENGINE_PLUGIN_ID;
	boost::shared_ptr<SpacePlugin> sp(new SpacePlugin(spId));
	Property::PluginMapT pluginMap;
	pluginMap.insert(sp);

	// Environment, GOF, Sector setup
	auto environment = boost::make_shared<Environment>();
	auto gof = boost::make_shared<GameObjectFactory>(lane, lc, pluginMap, environment, generateHandle());
	auto sector = boost::make_shared<Sector>(lane, 1, "TestSector", gof);
	
	// Create an object (with some 'damage' as goValue)
	const BFG::s32 damage = 50;
	ObjectParameter op;
	op.mHandle = generateHandle();
	op.mName = "Test Object";
	op.mType = "Cube";
	op.mGoValues[ValueId(ID::PV_Damage, spId)] = damage;
	lane.emit(ID::S_CREATE_GO, op);
	
	sync.start();
	sync.finish();
	
	// It must exist
	BOOST_REQUIRE(environment->exists(op.mHandle));
	
	// It must contain the value 
	BOOST_CHECK_EQUAL(environment->getGoValue<BFG::s32>(op.mHandle, ID::PV_Damage, spId), damage);
}

BOOST_AUTO_TEST_CASE (AddRemoveTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	// Sector prerequisites
	BFG::Path p;
	std::string def = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "default/";
	
	BFG::LevelConfig lc;
	lc.mModules.push_back(def + "Object.xml");
	lc.mAdapters.push_back(def + "Adapter.xml");
	lc.mConcepts.push_back(def + "Concept.xml");
	lc.mProperties.push_back(def + "Value.xml");

	BFG::Property::PluginMapT pluginMap;
	auto go = createTestGameObject(lane, pluginMap).first;
	auto gof = boost::make_shared<BFG::GameObjectFactory>(lane, lc, pluginMap, go->environment(), BFG::generateHandle());
	
	// Create a sector
	BFG::Sector sector(lane, 1, "TestSector", gof);
	
	// Adding an object
	sector.addObject(go);
	
	// Adding the same object again
	BOOST_CHECK_THROW(sector.addObject(go), std::logic_error);
	
	// Mark it for removal
	sector.removeObject(go->getHandle());
	
	// Shouldn't be removed yet.
	BOOST_CHECK_THROW(sector.addObject(go), std::logic_error);
	
	// The object will be removed at the next update() call
	sector.update(1.0f * si::seconds);
	
	// Adding the object again after removal should be OK
	BOOST_CHECK_NO_THROW(sector.addObject(go));
}

BOOST_AUTO_TEST_SUITE_END()
