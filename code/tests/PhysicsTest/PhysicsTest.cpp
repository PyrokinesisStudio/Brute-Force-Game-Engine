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

#ifndef dSINGLE
#define dSINGLE
#endif


#include <boost/array.hpp>
#include <boost/test/unit_test.hpp>

#include <ode/ode.h>

#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Base/PeriodicWaitForEqual.h>

#include <Core/Math.h>

#include <Event/Event.h>
#include <Event/Catcher.h>

#include <Physics/Event_fwd.h>
#include <Physics/Main.h>
#include <Physics/PhysicsObject.h>

#include <View/MiniMain.h>

typedef BFG::Event::Catcher<BFG::Physics::FullSyncData> FullSyncDataCatcherT;

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(PhysicsTestSuite)

BOOST_AUTO_TEST_CASE (testLibraryInit)
{
	// Logger init may be done within the first test.
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "PhysicsTest.log");
	
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100);
	
	physicsLane.addEntry<BFG::Physics::Main>();

	sync.start();
	sync.finish();
}

BOOST_AUTO_TEST_CASE (testCreateOneObject)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100);

	physicsLane.addEntry<BFG::Physics::Main>();
	
	sync.start();

	// BUG: rootModule is zero if no module has been added yet.
	BFG::GameHandle rootModule = 0;
	
	FullSyncDataCatcherT catcher(physicsLane, BFG::ID::PE_FULL_SYNC, rootModule);
	
	BFG::GameHandle handle = BFG::generateHandle();
	BFG::Location location(v3::NEGATIVE_UNIT_Z);
	
	BFG::Physics::ObjectCreationParams ocp(handle, location);
	physicsLane.emit(BFG::ID::PE_CREATE_OBJECT, ocp);

	sync.finish();

	BOOST_REQUIRE_EQUAL(catcher.count(), 1);
	auto fstPayload = catcher.payloads().at(0);
	BOOST_CHECK(BFG::nearEnough(fstPayload.get<0>(), v3::NEGATIVE_UNIT_Z, 0.001f));
}

BOOST_AUTO_TEST_CASE (testCreateOneObjectWithOneModule)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100, "Physics", BFG::Event::RL1);
	BFG::Event::Lane miniView(sync, 100, "View", BFG::Event::RL2);
	
	physicsLane.addEntry<BFG::Physics::Main>();
	miniView.addEntry<BFG::View::MiniMain>();
	
	sync.start();
	
	BFG::GameHandle handle = BFG::generateHandle();
	FullSyncDataCatcherT catcher(physicsLane, BFG::ID::PE_FULL_SYNC, handle);

	// Create Physics Object
	BFG::Location location;
	BFG::Physics::ObjectCreationParams ocp(handle, location);
	physicsLane.emit(BFG::ID::PE_CREATE_OBJECT, ocp);

	// Create Physics Module
	BFG::Physics::ModuleCreationParams mcp
	(
		handle,
		handle,
		"Cube.mesh",
		5.0f,
		BFG::ID::CM_Standard,
		v3::UNIT_X
	);
	physicsLane.emit(BFG::ID::PE_ATTACH_MODULE, mcp);
	sync.finish();
	
	BOOST_CHECK_EQUAL(catcher.count(), 1);
	auto fstPayload = catcher.payloads().at(0);
	BOOST_CHECK(BFG::nearEnough(fstPayload.get<0>(), v3::UNIT_X, 0.001f));
}

BOOST_AUTO_TEST_CASE (testCreateOneObjectWithTwoModulesAndSetValues)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane physicsLane(sync, 100, "Physics", BFG::Event::RL1);
	BFG::Event::Lane miniView(sync, 100, "View", BFG::Event::RL2);

	physicsLane.addEntry<BFG::Physics::Main>();
	miniView.addEntry<BFG::View::MiniMain>();

	sync.start();

	// BUG: rootModule is zero if no module has been added yet.
	BFG::GameHandle rootModule = BFG::generateHandle();
	FullSyncDataCatcherT catcher(physicsLane, BFG::ID::PE_FULL_SYNC, rootModule);

	// Create an empty physics module. Also, create the root physics module.
	// Its parameters will define the physical start values of the whole
	// object.
	
	BFG::GameHandle handle = rootModule;

	// Empty physics object
	BFG::Physics::ObjectCreationParams ocp(handle, BFG::Location());
	physicsLane.emit(BFG::ID::PE_CREATE_OBJECT, ocp);

	// First module (root module)
	auto expectedRootPos = v3::UNIT_X;
	auto expectedRootOri = BFG::qv4(0,1,0,0);
	auto expectedRootVel = BFG::v3::ZERO;
	auto expectedRootRotVel = BFG::v3::ZERO;
	
	BFG::Physics::ModuleCreationParams mcp
	(
		handle,
		handle,
		"Cube.mesh",
		5.0f,
		BFG::ID::CM_Standard,
		expectedRootPos,
		expectedRootOri,
		expectedRootVel,
		expectedRootRotVel
	);
	physicsLane.emit(BFG::ID::PE_ATTACH_MODULE, mcp);
	
	// By now, exactly one FullSyncData event should have been arrived.
	BFG::Base::periodicWaitForEqual(catcher.count(), 1, boost::posix_time::milliseconds(500));
	BOOST_REQUIRE_EQUAL(catcher.count(), 1);

	auto fstFullSyncData = catcher.payloads()[0];
	auto receivedPos = fstFullSyncData.get<0>();
	auto receivedOri = fstFullSyncData.get<1>();
	auto receivedVel = fstFullSyncData.get<2>();
	auto receivedRotVel = fstFullSyncData.get<4>();
	
	BOOST_CHECK(BFG::nearEnough(receivedPos, expectedRootPos, 0.001f));
	BOOST_CHECK(BFG::equals(receivedOri, expectedRootOri));
	BOOST_CHECK(BFG::nearEnough(receivedVel, expectedRootVel, 0.001f));
	BOOST_CHECK(BFG::nearEnough(receivedRotVel, expectedRootRotVel, 0.001f));

	// Now we'll create the second module. It will emit another FullSyncData
	// event. The transmitted start values are relative to the first module
	// now. Since FullSyncData contains values based on the root module it
	// will still contain the same values.
	
	auto expectedSndModulePos = v3::UNIT_Y;
	
	// Create the second physics module
	BFG::Physics::ModuleCreationParams mcp2
	(
		handle,
		BFG::generateHandle(),
		"Cube.mesh",
		10.0f,
		BFG::ID::CM_Standard,
		v3::UNIT_Y
	);
	physicsLane.emit(BFG::ID::PE_ATTACH_MODULE, mcp2);
	
	// By now, exactly two FullSyncData events should have been arrived.
	BFG::Base::periodicWaitForEqual(catcher.count(), 2, boost::posix_time::milliseconds(500));
	BOOST_REQUIRE_EQUAL(catcher.count(), 2);

	fstFullSyncData = catcher.payloads()[1];
	receivedPos = fstFullSyncData.get<0>();
	receivedOri = fstFullSyncData.get<1>();
	receivedVel = fstFullSyncData.get<2>();
	receivedRotVel = fstFullSyncData.get<4>();

	// As mentioned above, it still should contain the same values since
	// the root module is used for FullSyncData.
	BOOST_CHECK(BFG::nearEnough(receivedPos, expectedRootPos, 0.001f));
	BOOST_CHECK(BFG::equals(receivedOri, expectedRootOri));
	BOOST_CHECK(BFG::nearEnough(receivedVel, expectedRootVel, 0.001f));
	BOOST_CHECK(BFG::nearEnough(receivedRotVel, expectedRootRotVel, 0.001f));

	// Now, let's set some values for the whole object (root module).
	BFG::Event::Catcher<BFG::v3> posCatcher(physicsLane, BFG::ID::PE_POSITION, handle);
	BFG::Event::Catcher<BFG::qv4> oriCatcher(physicsLane, BFG::ID::PE_ORIENTATION, handle);

	// Set some values
	auto newPos = v3::NEGATIVE_UNIT_X;
	auto newOri = BFG::qv4(0,1,0,0);
	auto newVel = BFG::v3::NEGATIVE_UNIT_Y;
	auto newRotVel = BFG::v3::NEGATIVE_UNIT_Z;
	
	physicsLane.emit(BFG::ID::PE_UPDATE_POSITION, newPos, handle);
	physicsLane.emit(BFG::ID::PE_UPDATE_ORIENTATION, newOri, handle);

	sync.finish();
	
	// No further FullSyncData events should have been received.
	BOOST_CHECK_EQUAL(catcher.count(), 2);
	
	// But exactly one PE_POSITION and one PE_ORIENTATION event
	BOOST_REQUIRE_NE(posCatcher.count(), 0);
	BOOST_REQUIRE_NE(oriCatcher.count(), 0);

	// Use latest data
	receivedPos = posCatcher.payloads().back();
	receivedOri = oriCatcher.payloads().back();
	BOOST_CHECK(BFG::nearEnough(receivedPos, newPos, 0.001f));
	BOOST_CHECK(BFG::equals(receivedOri, newOri));
}

BOOST_AUTO_TEST_CASE (interpolatorTest)
{
	BFG::Physics::Interpolator interpolator;

	BFG::u32 timeStamp = 0; // not used
	BFG::u16 age = 500; // ms old
	BFG::v3 lastPosition(0.0f, 0.0f, 0.0f); // last recorded position

	BFG::v3 velocity(10.0f, 0.0f, 0.0f); // current velocity
	BFG::v3 currentPosition(4.0f, 0.0f, 0.0f);

	BFG::Physics::InterpolationDataV3 interpolationDataV3(timeStamp, age, lastPosition);

	interpolator.preparePosition(interpolationDataV3, currentPosition, velocity);

	BOOST_CHECK(BFG::nearEnough(interpolator.mEndPosition, v3(5.0f, 0.0f, 0.0f), BFG::EPSILON_F));

	bool interpolated = interpolator.interpolatePosition(0.1f);

	BOOST_CHECK_EQUAL(interpolated, true);
	BOOST_CHECK_CLOSE(interpolator.mPositionParameter, 0.1f, BFG::EPSILON_F);
	BOOST_CHECK(BFG::nearEnough(interpolator.mInterpolatedPosition, v3(4.1f, 0.0f, 0.0f), BFG::EPSILON_F));

	// check orientation interpolation
// 	BFG::qv4 lastOrientation(BFG::qv4::IDENTITY);
// 
// 	BFG::Physics::InterpolationDataQv4 interpolationDataQv4(timeStamp, age, lastOrientation);
// 
// 	BFG::v3 rotVelocity(45.0f * DEG2RAD, 0.0f, 0.0f);
// 	BFG::qv4 currentOrientation(BFG::eulerToQuaternion(BFG::v3(15.0f * DEG2RAD, 0.0f, 0.0f)));
// 
// 	interpolator.prepareOrientation(interpolationDataQv4, currentOrientation, rotVelocity);
// 
// 	BOOST_CHECK_EQUAL(BFG::equals(interpolator.mEndOrientation, BFG::eulerToQuaternion(BFG::v3(22.5f * DEG2RAD, 0.0f, 0.0f)), BFG::EPSILON_F), true);
// 
// 	interpolated = interpolator.interpolateOrientation(0.1f);
// 	BOOST_CHECK_EQUAL(interpolated, true);
// 	BOOST_CHECK_CLOSE(interpolator.mOrientationParameter, 0.1f, BFG::EPSILON_F);
// 	BOOST_CHECK_EQUAL(BFG::equals(interpolator.mInterpolatedOrientation, BFG::eulerToQuaternion(BFG::v3(18.75f * DEG2RAD, 0.0f, 0.0f)), BFG::EPSILON_F), true);
}
BOOST_AUTO_TEST_SUITE_END()
