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

#define BOOST_TEST_MODULE EventTest

#include <boost/assign/std/vector.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/mutex.hpp>

#include <Base/Logger.h>

#include <Core/GameHandle.h>

#include <Event/Binding.h>
#include <Event/Event.h>

//! Handy shortcut for BOOST_CHECK_EQUAL_COLLECTIONS
#ifndef CHECK_EQUAL_COLLECTIONS
#define CHECK_EQUAL_COLLECTIONS(a,b) \
	BOOST_CHECK_EQUAL_COLLECTIONS((a).begin(), (a).end(), (b).begin(), (b).end())
#endif

const int testEventId = 5;
const BFG::GameHandle testDestinationId = 15;
const BFG::GameHandle testDestinationId2 = 16;
const BFG::GameHandle testSenderId = 5678;
const BFG::GameHandle testSenderId2 = 91011;

static BFG::u32 gf32EventCounter = 0;
static BFG::u32 gu32EventCounter = 0;
static BFG::u32 gVoidEventCounter = 0;

static std::vector<std::vector<BFG::u32> > gVectorEventCounter;

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
	void run(BFG::Event::Lane*)
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
	void run(BFG::Event::Lane*)
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

struct TestModule3 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connect(1, this, &TestModule3::testEventHandler);
	}

	void testEventHandler(const BFG::u32&)
	{
		boost::mutex::scoped_lock(mMutex);
		++gu32EventCounter;
	}

	boost::mutex mMutex;
};

struct TestModule4 : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	TestModule4(BFG::u32 frequency) :
	mFrequency(frequency)
	{}

	void run(LaneT* lane)
	{
		lane->connectV(1, this, &TestModule4::handleVoidEvent);
	}

	void handleVoidEvent()
	{
		gVoidEventCounter = mFrequency;
	}

	BFG::u32 mFrequency;
};

struct TestModuleCopy : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connect(3, this, &TestModuleCopy::testEventHandler);
	}

	void testEventHandler(BFG::u32)
	{
		boost::mutex::scoped_lock(mMutex);
		++gu32EventCounter;
	}
};

struct TestModuleConst : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connect(4, this, &TestModuleConst::testEventHandler);
	}

	void testEventHandler(BFG::u32) const
	{
		boost::mutex::scoped_lock(mMutex);
		++gu32EventCounter;
	}
};

struct ThrowingModule : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connectV(5, this, &ThrowingModule::testEventHandler);
	}

	void testEventHandler() const
	{
		throw std::runtime_error("ThrowingModule Test");
	}
};

struct TestContainerModule : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		lane->connect(10001, this, &TestContainerModule::testEventHandler);
	}

	void testEventHandler(const std::vector<BFG::u32>& cont)
	{
		gVectorEventCounter.push_back(cont);
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestSuite)

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

	sync.start();

	sync.finish();
}

BOOST_AUTO_TEST_CASE (CopyParameter)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	lane.addEntry<TestModuleCopy>();
	gu32EventCounter = 0;

	sync.start();

	BFG::u32 value = 1;
	lane.emit(3, value);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gu32EventCounter, 1);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gu32EventCounter, 1);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (ConstFunction)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	lane.addEntry<TestModuleConst>();
	gu32EventCounter = 0;

	sync.start();

	BFG::u32 value = 1;
	lane.emit(4, value);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gu32EventCounter, 1);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gu32EventCounter, 1);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (OneLaneWithOneEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	TestModule t;
	gf32EventCounter = 0;

	lane.connect(1, &t, &TestModule::testEventHandler);

	sync.start();

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

	sync.start();

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

	sync.start();

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

	sync.start();

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

	sync.start();

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

	sync.start();

	lane1.emit(1, 1.0f);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 2);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 2);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (TwoLanesTwoEntryPointsTwoHandlerWithDifferentPayloadsForSameEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane1(sync, 100);
	BFG::Event::Lane lane2(sync, 100);

	gf32EventCounter = 0;
	gu32EventCounter = 0;

	lane1.addEntry<TestModule>();
	lane2.addEntry<TestModule3>(); // shouldn't this cause an IncompatibleTypeException?

	sync.start();

	// this causes one IncompatibleTypeException
	BOOST_REQUIRE_THROW(lane1.emit(1, 1.0f), BFG::Event::IncompatibleTypeException);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);
	BOOST_REQUIRE_EQUAL(gu32EventCounter, 0);

	boost::this_thread::sleep(boost::posix_time::millisec(100));

	BOOST_REQUIRE_EQUAL(gf32EventCounter, 1);
	BOOST_REQUIRE_EQUAL(gu32EventCounter, 0);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (OneLaneOneEventWrongPayload)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	TestModule t;

	lane.connect(1, &t, &TestModule::testEventHandler);

	sync.start();

	BOOST_REQUIRE_THROW(lane.emit(1, std::string("Hello")), BFG::Event::IncompatibleTypeException);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (OneLaneOneEventWithException)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	lane.addEntry<ThrowingModule>();
	sync.start();
	
	lane.emit(5, BFG::Event::Void());
	sync.finish();
}

#if 0 // TODO
BOOST_AUTO_TEST_CASE (SynchronizerWithoutFinish)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	sync.start();
}
#endif

BOOST_AUTO_TEST_CASE (EntryWithStartParameter)
{
	gVoidEventCounter = 0;

	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	BFG::u32 frequency = 100;
	lane.addEntry<TestModule4>(frequency);

	sync.start();

	lane.emit(1, BFG::Event::Void());
	sync.finish();

	BOOST_REQUIRE_EQUAL(gVoidEventCounter, frequency);
}

BOOST_AUTO_TEST_CASE (EmitStdContainer)
{

	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	BFG::u32 frequency = 100;
	lane.addEntry<TestContainerModule>();

	sync.start();
	
	using namespace boost::assign;

	std::vector<BFG::u32> testData1;
	testData1 += 92,83745,0,9823,740958,74,8237,40598,2730,5987,91287,54098,750,2934750,92,384,5702,9384,23,9480,23948,023,948,2348;

	std::vector<BFG::u32> testData2;
	testData2 += 55,43,24,6,2345,63456,34,5867,555,5678,365,896,24,574,5432,74,4567,256,347,373;

	std::vector<BFG::u32> testData3;
	testData3 += 82,4,0,9587,23,459,827,34,5,90823,74,5092,83,745092,834,7502,98,70764,1,89374,472,938,4750,298,34,7509,2834;

	lane.emit(10001, testData1);
	lane.emit(10001, testData2);
	lane.emit(10001, testData3);

	sync.finish();

	CHECK_EQUAL_COLLECTIONS(gVectorEventCounter[0], testData1);
	CHECK_EQUAL_COLLECTIONS(gVectorEventCounter[1], testData2);
	CHECK_EQUAL_COLLECTIONS(gVectorEventCounter[2], testData3);
}


#if 0
BOOST_AUTO_TEST_CASE (TwoConnectsSameEvent){}
BOOST_AUTO_TEST_CASE (TwoConnectsSameEventButDifferentPayloads){}
BOOST_AUTO_TEST_CASE (TwoLanesOneConnectPerLaneSameEvent){}
BOOST_AUTO_TEST_CASE (TwoLanesOneConnectPerLaneSameEventButDifferentPayload){}
BOOST_AUTO_TEST_CASE (OneLaneWithoutFinish){}
#endif



BOOST_AUTO_TEST_SUITE_END()
