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

#include <EventSystem/Emitter.h>
#include <EventSystem/Core/CommunicationPolicy.h>
#include <EventSystem/Core/ThreadingPolicy.h>

#include "EventCounter.h"

#include <boost/test/unit_test.hpp>

// Toggle
#define SKIP_BROKEN_TESTS


#ifndef SKIP_BROKEN_TESTS
static void manyLoops(const std::string& name, IEventLoop* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);
	for (size_t i=0; i<10000000; ++i)
	{
		long processedEvents = loop->doLoop();
		if (processedEvents > 0)
			std::cout << "Processed " << processedEvents << " events.\n";
	}
	std::cout << name << " done.\n";
}
#endif

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(MultiThreadTestSuite)
#if 0
BOOST_AUTO_TEST_CASE (TestTwoLoopsWithoutCommunication)
{
	EventLoop loop1(false);
	EventLoop loop2(false);
	
	BFG::Emitter e1(&loop1);
	BFG::Emitter e2(&loop2);
	
	TestEventCounter tec;

	e1.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);

	loop2.connect(TEST_EVENT_ID, &tec, &TestEventCounter::eventHandler);
	
	loop1.doLoop();
	loop2.doLoop();

	// Expect no received events since we arent't using "Interthread Communication"
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 0);
}

BOOST_AUTO_TEST_CASE (TestTwoLoopsWithOneWayCommunication)
{
	EventLoop loop1(false, new EventSystem::NoThread<>(), new EventSystem::InterThreadCommunication());
	EventLoop loop2(false);
	
	BFG::Emitter e1(&loop1);
	BFG::Emitter e2(&loop2);
	
	TestEventCounter tec;

	e1.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);

	loop2.connect(TEST_EVENT_ID, &tec, &TestEventCounter::eventHandler);
	
	loop1.doLoop();
	loop2.doLoop();

	// Expect no received events since loop2 has no communication and does
	// not poll if other pools are available.
	BOOST_REQUIRE_EQUAL(tec.receivedEvents(), 0);
}

#endif

#ifndef SKIP_BROKEN_TESTS
BOOST_AUTO_TEST_CASE (TestTwoLoopsWithCommunication)
{
	EventLoop loop1(false, new EventSystem::BoostThread<>("Loop1", &manyLoops), new EventSystem::InterThreadCommunication());
	EventLoop loop2(false, new EventSystem::BoostThread<>("Loop2", &manyLoops), new EventSystem::InterThreadCommunication());
	
	BFG::Emitter e1(&loop1);
	BFG::Emitter e2(&loop2);
	
	TestEventCounter tec;

	// Sending 1000 events with Loop1 to an event handler of Loop2.
	
	loop2.connect(TEST_EVENT_ID, &tec, &TestEventCounter::eventHandler);

	loop1.run();
	loop2.run();

	std::cout << "Sleep (100ms)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::microseconds(100));

	const size_t EXPECTED_NUMBER_OF_EVENTS = 1000;
	for (size_t i=0; i<EXPECTED_NUMBER_OF_EVENTS; ++i)
	{
		e1.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
	}

	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	loop1.setExitFlag();
	loop2.setExitFlag();

	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	
	std::cout << "Received Events: " << tec.receivedEvents() << std::endl;
	
	// Expect EXPECTED_NUMBER_OF_EVENTS received events. Both loops should communicate.
	BOOST_CHECK_EQUAL(tec.receivedEvents(), EXPECTED_NUMBER_OF_EVENTS);
}
#endif

#ifndef SKIP_BROKEN_TESTS
BOOST_AUTO_TEST_CASE (TestCreatePoolElementException)
{
	EventLoop loop1(false, new EventSystem::BoostThread<>("Loop1", &manyLoops), new EventSystem::InterThreadCommunication());
	
	BFG::Emitter e1(&loop1);
	
	TestEventCounter tec;

	// Provoking "unable to create pool element"
	
	loop1.connect(TEST_EVENT_ID, &tec, &TestEventCounter::eventHandler);
	loop1.run();

	std::cout << "Sleep (100ms)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::microseconds(100));

	const size_t EXPECTED_NUMBER_OF_EVENTS = 1000000;

	BOOST_CHECK_NO_THROW(
		for (size_t i=0; i<EXPECTED_NUMBER_OF_EVENTS; ++i)
		{
			e1.emit<TestEvent>(TEST_EVENT_ID, TEST_EVENT_PAYLOAD);
		}
	);

	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	loop1.setExitFlag();

	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	std::cout << "Sleep (1)" << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	
	std::cout << "Received Events: " << tec.receivedEvents() << std::endl;
	
	// Expect EXPECTED_NUMBER_OF_EVENTS received events without error.
	BOOST_CHECK_EQUAL(tec.receivedEvents(), EXPECTED_NUMBER_OF_EVENTS);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
