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

#include <Controller/Controller.h>
#include <Event/Event.h>
#include <Event/Catcher.h>

#define BOOST_TEST_MODULE ControllerTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(ControllerTestSuite)

BOOST_AUTO_TEST_CASE (ActionBuilderTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	BFG::Event::Catcher<BFG::Controller_::ActionDefinition> actionCatcher
	(
		lane,
		BFG::ID::CE_ADD_ACTION,
		0
	);
	
	const BFG::Event::IdT myEventId = 123;
	BFG::Controller_::ActionMapT myActionMap;
	myActionMap[456] = "456";
	
	// Actual usage here
	auto subLane = lane.createSubLane();
	auto ab = BFG::Controller_::createActionBuilder(subLane);
	ab.addDefaultActions();
	ab.add(myEventId, "MY_EVENT");
	ab.add(myActionMap);
	ab.emitActions();
	
	// Test
	sync.start();
	sync.finish();
	
	BOOST_REQUIRE_NE(actionCatcher.count(), 0);
	
	auto p = actionCatcher.payloads();
	
	auto containsActionId = [&](BFG::Event::IdT id){
		for (auto actionDefinition : p)
			if (actionDefinition.get<0>() == id)
				return true;
		return false;
	};
	
	// Should contain action from simple add()
	BOOST_CHECK(containsActionId(123));
	
	// Should contain action from own ActionMap
	BOOST_CHECK(containsActionId(456));
	
	// Should contain default actions
	BOOST_CHECK(containsActionId(BFG::ID::A_MOUSE_MOVE_X));
	BOOST_CHECK(containsActionId(BFG::ID::A_MOUSE_MOVE_Y));
	BOOST_CHECK(containsActionId(BFG::ID::A_MOUSE_MOVE_Z));
	BOOST_CHECK(containsActionId(BFG::ID::A_MOUSE_LEFT_PRESSED));
	BOOST_CHECK(containsActionId(BFG::ID::A_MOUSE_MIDDLE_PRESSED));
	BOOST_CHECK(containsActionId(BFG::ID::A_MOUSE_RIGHT_PRESSED));
	BOOST_CHECK(containsActionId(BFG::ID::A_KEY_PRESSED));
	BOOST_CHECK(containsActionId(BFG::ID::A_KEY_RELEASED));
	
	BOOST_CHECK_EQUAL(actionCatcher.count(), 10);
}

BOOST_AUTO_TEST_SUITE_END()
