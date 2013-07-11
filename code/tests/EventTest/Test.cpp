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

#include <Core/GameHandle.h>

#include <Event/Event.h>

#include <boost/test/unit_test.hpp>
#include <boost/thread/mutex.hpp>

const int testEventId = 5;
const BFG::GameHandle testDestinationId = 15;
const BFG::GameHandle testDestinationId2 = 16;
const BFG::GameHandle testSenderId = 5678;
const BFG::GameHandle testSenderId2 = 91011;

static BFG::u32 gf32EventCounter = 0;
static BFG::u32 gu32EventCounter = 0;

struct TestModulAudio1 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		mLane = lane;
		std::cout << "TestModulAudio1-Run" << std::endl;
		mLane->connect(1, this, &TestModulAudio1::event1Handler);
		mLane->connectLoop(this, &TestModulAudio1::update);
	}

	void stop()
	{
		std::cout << "TestModulAudio1-Stop" << std::endl;
	}

	void update(const BFG::Event::TickData ld)
	{
		std::cout << "TestModulAudio1-update(" << ld.mTimeSinceLastTick << ")" << std::endl;
		mLane->emit(2, std::string("a1Emit2"));
	}

	void event1Handler(const std::string& s, const BFG::GameHandle& sender)
	{
		std::cout << "TestModulAudio1: from (" << BFG::stringify(sender) << ") , " << s << std::endl;
	}

	BFG::Event::Lane* mLane;
};

struct TestModulAudio2 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		std::cout << "TestModulAudio2-Run" << std::endl;
	}
};

struct TestModulView1 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		mLane = lane;
		std::cout << "TestModulView1-Run" << std::endl;

		mLane->connect(2, this, &TestModulView1::event2Handler);
	}

	void event2Handler(const std::string& s)
	{
		std::cout << "TestModulView1: " << s << std::endl;
		mLane->emit(1, std::string("v1Emit1"));
	}

	BFG::Event::Lane* mLane;
};

struct TestModulPhysics1 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		mLane = lane;
		std::cout << "TestModulPhysics1-Run" << std::endl;

		mLane->connect(2, this, &TestModulPhysics1::event2Handler);
	}

	void stop()
	{
		std::cout << "TestModulPhysics1-Stop" << std::endl;
	}

	void event2Handler(const std::string& s, const BFG::GameHandle& sender)
	{
		std::cout << "TestModulPhysics1: from (" << BFG::stringify(sender) << ") , " << s << std::endl;
	}

	BFG::Event::Lane* mLane;
};

struct TestModulGame1 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		std::cout << "TestModulGame1-Run" << std::endl;
	}
};

struct TestModule : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connect(1, this, &TestModule::testEventHandler);
	}

	void testEventHandler(const BFG::f32&)
	{
		boost::mutex::scoped_lock(mMutex);
		++gf32EventCounter;
	}

	boost::mutex mMutex;
};

struct TestModule2 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connect(2, this, &TestModule2::testEventHandler);
	}

	void testEventHandler(const BFG::u32&)
	{
		boost::mutex::scoped_lock(mMutex);
		++gu32EventCounter;
	}

	boost::mutex mMutex;
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestSuite)

#if 0
BOOST_AUTO_TEST_CASE (Test)
{
	const int ticksPerSecond100 = 100;
	const int ticksPerSecond5 = 5;
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane audio(sync, ticksPerSecond5);
	BFG::Event::Lane physics(sync, ticksPerSecond100);
	BFG::Event::Lane view(sync, ticksPerSecond100);
	BFG::Event::Lane game(sync, ticksPerSecond100);

	// EntryPoints

	audio.addEntry<TestModulAudio1>();
	audio.addEntry<TestModulAudio2>();
	physics.addEntry<TestModulPhysics1>();
	view.addEntry<TestModulView1>();
	game.addEntry<TestModulGame1>();

	sync.startEntries();

	sync.finish();
}
#endif

BOOST_AUTO_TEST_CASE (OneLaneWithOneEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	TestModule t;
	gf32EventCounter = 0;

	lane.connect(1, &t, &TestModule::testEventHandler);

	sync.startEntries();

	lane.emit(1, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (OneLaneWithOneUnconnectedEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	TestModule t;
	gf32EventCounter = 0;

	lane.connect(1, &t, &TestModule::testEventHandler);

	sync.startEntries();

	lane.emit(2, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 0);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 0);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (OneLaneWithEntryPointAndOneEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	gf32EventCounter = 0;

	lane.addEntry<TestModule>();

	sync.startEntries();

	lane.emit(1, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (OneLaneTwoEntryPointOneEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	gf32EventCounter = 0;

	lane.addEntry<TestModule>();
	lane.addEntry<TestModule>();

	sync.startEntries();

	lane.emit(1, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 2);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 2);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (TwoLanesOneEntryPointOneEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane1(sync, 100);
	BFG::Event::Lane lane2(sync, 100);

	gf32EventCounter = 0;

	lane2.addEntry<TestModule>();

	sync.startEntries();

	lane1.emit(1, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (TwoLanesTwoEntryPointsOneEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane1(sync, 100);
	BFG::Event::Lane lane2(sync, 100);

	gf32EventCounter = 0;

	lane1.addEntry<TestModule>();
	lane2.addEntry<TestModule>();

	sync.startEntries();

	lane1.emit(1, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 2);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 2);

	sync.finish();
}

#if 0

BOOST_AUTO_TEST_CASE (TwoConnectsSameEvent){}
BOOST_AUTO_TEST_CASE (TwoConnectsSameEventButDifferentPayloads){}
BOOST_AUTO_TEST_CASE (TwoLanesOneConnectPerLaneSameEvent){}
BOOST_AUTO_TEST_CASE (TwoLanesOneConnectPerLaneSameEventButDifferentPayload){}

// This won't work because the required exception is caught in Binder.
BOOST_AUTO_TEST_CASE (OneLaneWithOneEventWithTheWrongPayload)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	TestModule t;

	lane.connect(1, &t, &TestModule::testEventHandler);

	sync.startEntries();

	BOOST_REQUIRE_THROW(lane.emit(1, std::string("Hello")), BFG::Event::IncompatibleTypeException);

	sync.finish();
}
#endif

BOOST_AUTO_TEST_SUITE_END()
