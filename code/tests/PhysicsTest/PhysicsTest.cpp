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

#define BOOST_TEST_MODULE PhysicsTest
#include <boost/test/unit_test.hpp>

#include <Event/Event.h>
#include <Physics/Main.h>
#include <Physics/Event_fwd.h>
#include <Core/Math.h>
#include <Base/Pause.h>

// ---------------------------------------------------------------------------

struct PhysicsClient
{
	PhysicsClient() :
	mCount(0)
	{}

	void onFullSync(const BFG::Physics::FullSyncData& fsd)
	{
		++mCount;
		mFsd = fsd;
	}
	
	BFG::u32 mCount;
	BFG::Physics::FullSyncData mFsd;
};

BOOST_AUTO_TEST_SUITE(PhysicsTestSuite)

BOOST_AUTO_TEST_CASE (testLibraryInit)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100);
	
	physicsLane.addEntry<BFG::Physics::Main>();

	sync.startEntries();
	sync.finish();
}

BOOST_AUTO_TEST_CASE (testCreateOneObject)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100);

	PhysicsClient pc;
	
	// BUG: rootModule is zero if no module has been added yet.
	BFG::GameHandle rootModule = 0;
	
	physicsLane.connect(BFG::ID::PE_FULL_SYNC, &pc, &PhysicsClient::onFullSync, rootModule);
	physicsLane.addEntry<BFG::Physics::Main>();
	
	sync.startEntries();

	BFG::GameHandle handle = BFG::generateHandle();
	BFG::Location location(v3::NEGATIVE_UNIT_Z);
	
	BFG::Physics::ObjectCreationParams ocp(handle, location);
	physicsLane.emit(BFG::ID::PE_CREATE_OBJECT, ocp);

	sync.finish();
	BOOST_CHECK_EQUAL(pc.mCount, 1);
	BOOST_CHECK(BFG::nearEnough(pc.mFsd.get<0>(), v3::NEGATIVE_UNIT_Z, 0.001f));
}

BOOST_AUTO_TEST_CASE (testCreateOneObjectWithOneModule)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100);

	BFG::GameHandle handle = BFG::generateHandle();
	PhysicsClient pc;
	
	physicsLane.connect(BFG::ID::PE_FULL_SYNC, &pc, &PhysicsClient::onFullSync, handle);
	physicsLane.addEntry<BFG::Physics::Main>();
	
	sync.startEntries();

	// Create Physics Object
	BFG::Location location;
	BFG::Physics::ObjectCreationParams ocp(handle, location);
	physicsLane.emit(BFG::ID::PE_CREATE_OBJECT, ocp);
	
	// Create Physics Module
	BFG::Physics::ModuleCreationParams mcp
	(
		handle,
		handle,
		"Blah.mesh",
		BFG::ID::CM_Standard,
		v3::UNIT_X,
		BFG::qv4::IDENTITY,
		5.0f
	);
	physicsLane.emit(BFG::ID::PE_ATTACH_MODULE, mcp);

	sync.finish();
	BOOST_CHECK_EQUAL(pc.mCount, 1);
	BOOST_CHECK(BFG::nearEnough(pc.mFsd.get<0>(), v3::ZERO, 0.001f));
}

BOOST_AUTO_TEST_CASE (testCreateOneObjectWithOneModuleAndSetValues)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100);

	PhysicsClient pc;
	
	// BUG: rootModule is zero if no module has been added yet.
	BFG::GameHandle rootModule = BFG::generateHandle();
	physicsLane.connect(BFG::ID::PE_FULL_SYNC, &pc, &PhysicsClient::onFullSync, rootModule);
	physicsLane.addEntry<BFG::Physics::Main>();
	
	sync.startEntries();

	// Create Physics Object
	BFG::GameHandle handle = rootModule;
	BFG::Physics::ObjectCreationParams ocp(handle, BFG::Location());
	physicsLane.emit(BFG::ID::PE_CREATE_OBJECT, ocp);

	// Create Physics Module
	BFG::Physics::ModuleCreationParams mcp
	(
		handle,
		handle,
		"Blah.mesh",
		BFG::ID::CM_Standard,
		v3::UNIT_X,
		BFG::qv4::IDENTITY,
		5.0f
	);
	physicsLane.emit(BFG::ID::PE_ATTACH_MODULE, mcp);
	
	// Set some values
	physicsLane.emit(BFG::ID::PE_UPDATE_POSITION, v3::NEGATIVE_UNIT_X, handle);
	physicsLane.emit(BFG::ID::PE_UPDATE_ORIENTATION, BFG::qv4(0,1,0,0), handle);
	physicsLane.emit(BFG::ID::PE_UPDATE_VELOCITY, v3::NEGATIVE_UNIT_Y, handle);
	physicsLane.emit(BFG::ID::PE_UPDATE_ROTATION_VELOCITY, v3::NEGATIVE_UNIT_Z, handle);

	// Create Physics Module
	BFG::Physics::ModuleCreationParams mcp2
	(
		handle,
		BFG::generateHandle(),
		"Blub.mesh",
		BFG::ID::CM_Standard,
		v3::UNIT_Y,
		BFG::qv4::IDENTITY,
		10.0f
	);
	physicsLane.emit(BFG::ID::PE_ATTACH_MODULE, mcp2);
	
	sync.finish();
	BOOST_CHECK_EQUAL(pc.mCount, 1);

	std::cout << pc.mCount << std::endl;
	std::cout << pc.mFsd.get<0>() << std::endl;
	std::cout << pc.mFsd.get<1>() << std::endl;
	std::cout << pc.mFsd.get<2>() << std::endl;
	std::cout << pc.mFsd.get<4>() << std::endl;
	
	BOOST_CHECK(BFG::nearEnough(pc.mFsd.get<0>(), v3::NEGATIVE_UNIT_X, 0.001f));
	BOOST_CHECK(BFG::equals(pc.mFsd.get<1>(), BFG::qv4(0,1,0,0)));
	BOOST_CHECK(BFG::nearEnough(pc.mFsd.get<2>(), v3::NEGATIVE_UNIT_Y, 0.001f));
	BOOST_CHECK(BFG::nearEnough(pc.mFsd.get<4>(), v3::NEGATIVE_UNIT_Z, 0.001f));
}

BOOST_AUTO_TEST_SUITE_END()
