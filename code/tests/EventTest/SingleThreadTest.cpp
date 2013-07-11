/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#include <boost/bind.hpp>
#include <Base/EntryPoint.h>
#include <EventSystem/Emitter.h>

#include "EventCounter.h"

#define BOOST_TEST_MODULE EventTest
#include <boost/test/unit_test.hpp>

static size_t gEntryPointCalled = 0;

static void resetGlobals()
{
	gEntryPointCalled = 0;
}

// ---------------------------------------------------------------------------

static void* entryPoint(void*)
{
	++gEntryPointCalled;
	return 0;
}

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(SingleThreadTestSuite)
#if 0
BOOST_AUTO_TEST_CASE (TestLoopWithOneEvent)
{
	resetGlobals();
	
	EventLoop loop(false);
	BFG::Emitter e(&loop);
	TestEventCounter tec;
	
	loop.connect(TEST_EVENT_ID, &tec, &TestEventCounter::eventHandler);

	// Shouldn't get delivered yet
	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 0);

	// Should get delivered
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 1);

	// Make sure it doesn't get delivered again
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 1);
}

BOOST_AUTO_TEST_CASE (TestLoopWithOneEventAndDestinationAndSender)
{
	resetGlobals();
	
	const int OTHER_DESTINATION = 500;
	const int DESTINATION = 999;
	const int SENDER = 1000;

	EventLoop loop(false);
	BFG::Emitter e(&loop);
	TestEventCounter tec;

	loop.connect(TEST_EVENT_ID, &tec, &TestEventCounter::eventHandler, DESTINATION);

	// Should not arrive since the DESTINATION is different
	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD, OTHER_DESTINATION, SENDER);
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 0);

	// Should arrive since the DESTINATION is equal
	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD, DESTINATION, SENDER);
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 1);

	// Should not arrive since it has no DESTINATION (which means: to all)
	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 1);
}

BOOST_AUTO_TEST_CASE (TestEntryPoint)
{
	resetGlobals();

	EventLoop loop(false);

	BFG::Base::CEntryPoint ep(&entryPoint);

	// Shouldn't get entered yet
	loop.addEntryPoint(&ep);
	BOOST_REQUIRE_EQUAL(gEntryPointCalled, 0);

	// Shouldn't get entered yet
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(gEntryPointCalled, 0);

	// Should get entered now
	loop.stop();
	loop.entry();
	BOOST_REQUIRE_EQUAL(gEntryPointCalled, 1);

	// Should get entered again
	loop.stop();
	loop.entry();
	BOOST_REQUIRE_EQUAL(gEntryPointCalled, 2);
}

BOOST_AUTO_TEST_CASE (TestLoopEvent)
{
	resetGlobals();

	EventLoop loop(true);
	LoopEventCounter ler;
	
	// Shouldn't get delivered yet
	loop.registerLoopEventListener(&ler, &LoopEventCounter::eventHandler);
	BOOST_REQUIRE_EQUAL(ler.receivedEvents(), 0);

	// Should get delivered now
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(ler.receivedEvents(), 1);

	// Make sure it does get delivered again
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(ler.receivedEvents(), 2);

	// Make sure it doesn't get delivered again
	loop.unregisterLoopEventListener(&ler);
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(ler.receivedEvents(), 2);
}

BOOST_AUTO_TEST_CASE (TestBroadcastEvent)
{
	EventLoop loop(false);
	BFG::Emitter e(&loop);

	TestEventCounter ter1;
	TestEventCounter ter2;
	
	loop.connect(TEST_EVENT_ID, &ter1, &TestEventCounter::eventHandler);
	loop.connect(TEST_EVENT_ID, &ter2, &TestEventCounter::eventHandler);

	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);

	// Should get delivered twice
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(ter1.receivedEvents(), 1);
	BOOST_REQUIRE_EQUAL(ter2.receivedEvents(), 1);

	// Make sure it doesn't get delivered again
	loop.doLoop();
	BOOST_REQUIRE_EQUAL(ter1.receivedEvents(), 1);
	BOOST_REQUIRE_EQUAL(ter2.receivedEvents(), 1);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
