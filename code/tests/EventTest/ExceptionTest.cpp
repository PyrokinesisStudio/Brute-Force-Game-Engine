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
#include <boost/test/unit_test.hpp>

#include <Base/EntryPoint.h>
#include <EventSystem/Emitter.h>

#include "EventCounter.h"

// ---------------------------------------------------------------------------

static size_t gEntryPointCalled = 0;
static size_t gStdExceptionsCatched = 0;
static size_t gUnknownExceptionsCatched = 0;

static void resetGlobals()
{
	gEntryPointCalled = 0;
	gStdExceptionsCatched = 0;
	gUnknownExceptionsCatched = 0;
}

static void* throwingEntryPoint(void*)
{
	++gEntryPointCalled;
	
	// Throw for testing purposes
	throw std::runtime_error("Exception in entryPoint()");

	return 0;
}

static void* normalEntryPoint(void*)
{
	++gEntryPointCalled;
	return 0;
}

class TestExceptionPolicy : public EventSystem::ExceptionPolicy
{
public:
	virtual void onStdException(const std::exception& ex)
	{
		++gStdExceptionsCatched;
	}

	virtual void onUnknownException()
	{
		++gUnknownExceptionsCatched;
	}
};

class ThrowingTestEventCounter : public TestEventCounter
{
public:
	ThrowingTestEventCounter(EventLoop& loop) :
	mLoop(loop)
	{}
	
	virtual void eventHandler(TestEvent* e)
	{
		TestEventCounter::eventHandler(e);
		
		// This is the only way to exit mLoop->run()
		mLoop.setExitFlag();
		
		// Throw for testing purposes
		throw std::runtime_error("Exception in handleEvent()");
	}
	
	EventLoop& mLoop;
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ExceptionTestSuite)
#if 0
BOOST_AUTO_TEST_CASE (TestExceptionInEntryPoint)
{
	resetGlobals();
	
	EventLoop loop
	(
		false,
		new EventSystem::NoThread<>,
		new EventSystem::NoCommunication,
		new TestExceptionPolicy
	);

	BFG::Emitter e(&loop);

	ThrowingTestEventCounter tec(loop);
	BFG::Base::CEntryPoint tep(&throwingEntryPoint);
	BFG::Base::CEntryPoint nep(&normalEntryPoint);
	
	loop.addEntryPoint(&tep);
	loop.addEntryPoint(&nep);
	loop.connect(TEST_EVENT_ID, &tec, &ThrowingTestEventCounter::eventHandler);

	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
	
	// Nothing should have been happened yet.
	BOOST_CHECK_EQUAL(gEntryPointCalled, 0);
	BOOST_CHECK_EQUAL(tec.receivedEvents(), 0);
	BOOST_CHECK_EQUAL(gStdExceptionsCatched, 0);
	BOOST_CHECK_EQUAL(gUnknownExceptionsCatched, 0);

	loop.run();
	
	// 1. The throwingEntryPoint should have been called
	// 2. The normalEntryPoint should not have been called
	// 3. The event TEST_EVENT_ID should not have been delivered
	BOOST_CHECK_EQUAL(gEntryPointCalled, 1);
	BOOST_CHECK_EQUAL(tec.receivedEvents(), 0);
	BOOST_CHECK_EQUAL(gStdExceptionsCatched, 1);
	BOOST_CHECK_EQUAL(gUnknownExceptionsCatched, 0);
}

BOOST_AUTO_TEST_CASE (TestExceptionInEventHandler)
{
	resetGlobals();
	
	EventLoop loop
	(
		false,
		new EventSystem::NoThread<>,
		new EventSystem::NoCommunication,
		new TestExceptionPolicy
	);
	
	BFG::Emitter e(&loop);

	ThrowingTestEventCounter tec(loop);
	BFG::Base::CEntryPoint nep(&normalEntryPoint);
	
	loop.addEntryPoint(&nep);
	loop.addEntryPoint(&nep);
	loop.connect(TEST_EVENT_ID, &tec, &ThrowingTestEventCounter::eventHandler);

	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
	
	// Nothing should have been happened yet.
	BOOST_CHECK_EQUAL(gEntryPointCalled, 0);
	BOOST_CHECK_EQUAL(tec.receivedEvents(), 0);
	BOOST_CHECK_EQUAL(gStdExceptionsCatched, 0);
	BOOST_CHECK_EQUAL(gUnknownExceptionsCatched, 0);

	loop.run();
	
	// 1. The throwingEntryPoint should have been called
	// 2. The normalEntryPoint should not have been called
	// 3. The event TEST_EVENT_ID should not have been delivered
	BOOST_CHECK_EQUAL(gEntryPointCalled, 2);
	BOOST_CHECK_EQUAL(tec.receivedEvents(), 2);
	BOOST_CHECK_EQUAL(gStdExceptionsCatched, 2);
	BOOST_CHECK_EQUAL(gUnknownExceptionsCatched, 0);
	
	// Send another event now
	e.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);

	// and restart the loop
	loop.setExitFlag(false);
	loop.run();

	// 1. The throwingEntryPoint should have been called
	// 2. The normalEntryPoint should not have been called
	// 3. The event TEST_EVENT_ID should not have been delivered
	BOOST_CHECK_EQUAL(gEntryPointCalled, 4);
	BOOST_CHECK_EQUAL(tec.receivedEvents(), 3);
	BOOST_CHECK_EQUAL(gStdExceptionsCatched, 3);
	BOOST_CHECK_EQUAL(gUnknownExceptionsCatched, 0);
}


#endif

BOOST_AUTO_TEST_SUITE_END()
