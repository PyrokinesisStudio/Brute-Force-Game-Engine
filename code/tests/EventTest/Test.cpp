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

#include <Event/Binding.h>

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

struct ModuleConnect : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
 		lane->connect(1, this, &ModuleConnect::simpleCopy);
 		lane->connect(2, this, &ModuleConnect::simpleCopyConst);
 		lane->connect(3, this, &ModuleConnect::extendedCopy);
 		lane->connect(4, this, &ModuleConnect::extendedCopyConst);

		lane->connect(5, this, &ModuleConnect::simpleReference);
		lane->connect(6, this, &ModuleConnect::simpleReferenceConst);
		lane->connect(7, this, &ModuleConnect::extendedReference);
		lane->connect(8, this, &ModuleConnect::extendedReferenceConst);

		lane->connectV(9, this, &ModuleConnect::simpleVoid);
		lane->connectV(10, this, &ModuleConnect::simpleVoidConst);
		lane->connectV(11, this, &ModuleConnect::extendedVoid);
		lane->connectV(12, this, &ModuleConnect::extendedVoidConst);
	}

	void simpleCopy(BFG::u32 u){std::cout << "simpleCopy(" << u << ")" << std::endl;}
	void simpleCopyConst(BFG::u32 u) const {std::cout << "simpleCopyConst(" << u << ")" << std::endl;}
	void extendedCopy(BFG::u32 u, const BFG::GameHandle& sender){std::cout << "extendedCopy(" << u << "," << sender << ")" << std::endl;}
	void extendedCopyConst(BFG::u32 u, const BFG::GameHandle& sender) const {std::cout << "extendedCopyConst(" << u << "," << sender << ")" << std::endl;}

	void simpleReference(const std::string& s){std::cout << "simpleReference(" << s << ")" << std::endl;}
	void simpleReferenceConst(const std::string& s) const {std::cout << "simpleReferenceConst(" << s << ")" << std::endl;}
	void extendedReference(const std::string& s, const BFG::GameHandle& sender){std::cout << "extendedReference(" << s << "," << sender << ")" << std::endl;}
	void extendedReferenceConst(const std::string& s, const BFG::GameHandle& sender) const {std::cout << "extendedReferenceConst(" << s << "," << sender << ")" << std::endl;}

	void simpleVoid(){std::cout << "simpleVoid" << std::endl;}
	void simpleVoidConst() const {std::cout << "simpleVoidConst" << std::endl;}
	void extendedVoid(const BFG::GameHandle& sender){std::cout << "extendedVoid(" << sender << ")" << std::endl;}
	void extendedVoidConst(const BFG::GameHandle& sender) const {std::cout << "extendedVoidConst(" << sender << ")" << std::endl;}
};

struct SubModuleConnect : ModuleConnect
{
	void run(BFG::Event::Lane* l)
	{
		lane = l;
		sublane = l->createSubLane();
		
 		sublane->connect(1, this, &ModuleConnect::simpleCopy);
 		sublane->connect(2, this, &ModuleConnect::simpleCopyConst);
 		sublane->connect(3, this, &ModuleConnect::extendedCopy);
 		sublane->connect(4, this, &ModuleConnect::extendedCopyConst);

		sublane->connect(5, this, &ModuleConnect::simpleReference);
		sublane->connect(6, this, &ModuleConnect::simpleReferenceConst);
		sublane->connect(7, this, &ModuleConnect::extendedReference);
		sublane->connect(8, this, &ModuleConnect::extendedReferenceConst);

		sublane->connectV(9, this, &ModuleConnect::simpleVoid);
		sublane->connectV(10, this, &ModuleConnect::simpleVoidConst);
		sublane->connectV(11, this, &ModuleConnect::extendedVoid);
		sublane->connectV(12, this, &ModuleConnect::extendedVoidConst);
		
		sublane->connectV(13, this, &SubModuleConnect::onDestroy);
	}
	
	void onDestroy()
	{
		std::cout << "SubModuleConnect.onDestroy -> removeSubLane" << std::endl;
		lane->removeSubLane(sublane.get());
	}
	
	BFG::Event::Lane* lane;
	boost::shared_ptr<BFG::Event::SubLane> sublane;
};

// ---------------------------------------------------------------------------


BOOST_AUTO_TEST_SUITE(TestSuite)

BOOST_AUTO_TEST_CASE (SubLaneTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	boost::shared_ptr<BFG::Event::SubLane> sublane(lane.createSubLane());
	
	// EntryPoints
	lane.addEntry<SubModuleConnect>();
	sync.startEntries();
	
	sublane->emit(5, std::string("SubLane to SubLane"));
	sublane->emit(6, std::string("SubLane to SubLane (again)"));
	lane.emit(7, std::string("Lane to SubLane"));
	lane.emit(13, BFG::Event::Void());
	
	boost::this_thread::sleep(boost::posix_time::millisec(100));
	
	lane.emit(5, std::string("Should not be emitted"));
	
	sync.finish();
}

BOOST_AUTO_TEST_CASE (BindingTest)
{
	ModuleConnect mc;
	BFG::GameHandle senderHandle = 83;
	// copy
	BFG::Event::Binding<BFG::u32, BFG::GameHandle> bu;

	bu.connect(boost::bind(&ModuleConnect::simpleCopy, &mc, _1));
	bu.connect(boost::bind(&ModuleConnect::simpleCopyConst, &mc, _1));
	bu.connect(boost::bind(&ModuleConnect::extendedCopy, &mc, _1, _2));
	bu.connect(boost::bind(&ModuleConnect::extendedCopyConst, &mc, _1, _2));

	BFG::u32 u = 15;
	bu.emit(u, senderHandle);

	bu.call();

	// reference
	BFG::Event::Binding<std::string, BFG::GameHandle> bs;

	bs.connect(boost::bind(&ModuleConnect::simpleReference, &mc, _1));
	bs.connect(boost::bind(&ModuleConnect::simpleReferenceConst, &mc, _1));
	bs.connect(boost::bind(&ModuleConnect::extendedReference, &mc, _1, _2));
	bs.connect(boost::bind(&ModuleConnect::extendedReferenceConst, &mc, _1, _2));

	std::string s = "Hello";
	bs.emit(s, senderHandle);

	bs.call();

	// void
	BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle> bv;

	bv.connect(boost::bind(&ModuleConnect::simpleVoid, &mc));
	bv.connect(boost::bind(&ModuleConnect::simpleVoidConst, &mc));
	bv.connect(boost::bind(&ModuleConnect::extendedVoid, &mc, _2));
	bv.connect(boost::bind(&ModuleConnect::extendedVoidConst, &mc, _2));

	BFG::Event::Void v;
	bv.emit(v, senderHandle);

	bv.call();
}

BOOST_AUTO_TEST_CASE (ConnectionTestU32)
{
	ModuleConnect mc;
	BFG::u32 id = 32;
	BFG::GameHandle senderHandle = 123;
	BFG::GameHandle dest = BFG::NULL_HANDLE;

	BFG::Event::Callable* cu1 = new BFG::Event::Binding<BFG::u32, BFG::GameHandle>();

	BFG::Event::Connection<BFG::u32, BFG::GameHandle> con1 = {id, dest, cu1};

	BFG::Event::Binding<BFG::u32, BFG::GameHandle>* bu1 = static_cast<BFG::Event::Binding<BFG::u32, BFG::GameHandle>*>(cu1);

	bu1->connect(boost::bind(&ModuleConnect::simpleCopy, &mc, _1));
	bu1->connect(boost::bind(&ModuleConnect::simpleCopyConst, &mc, _1));
	bu1->connect(boost::bind(&ModuleConnect::extendedCopy, &mc, _1, _2));
	bu1->connect(boost::bind(&ModuleConnect::extendedCopyConst, &mc, _1, _2));

	BFG::Event::Callable* cu2 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<BFG::u32, BFG::GameHandle>* bu2 = static_cast<BFG::Event::Binding<BFG::u32, BFG::GameHandle>*>(cu2);

	BFG::u32 u = 55;
	bu2->emit(u, senderHandle);
	u = 43;
	bu2->emit(u, senderHandle);

	BFG::Event::Callable* cu3 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<BFG::u32, BFG::GameHandle>* bu3 = static_cast<BFG::Event::Binding<BFG::u32, BFG::GameHandle>*>(cu3);

	bu3->call();

	BFG::Event::Callable* cu4 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	delete cu4;
}

BOOST_AUTO_TEST_CASE (ConnectionTestString)
{
	ModuleConnect mc;
	BFG::u32 id = 18;
	BFG::GameHandle senderHandle = 225;
	BFG::GameHandle dest = BFG::NULL_HANDLE;

	BFG::Event::Callable* cs1 = new BFG::Event::Binding<std::string, BFG::GameHandle>();

	BFG::Event::Connection<BFG::u32, BFG::GameHandle> con1 = {id, dest, cs1};

	BFG::Event::Binding<std::string, BFG::GameHandle>* bs1 = static_cast<BFG::Event::Binding<std::string, BFG::GameHandle>*>(cs1);

	bs1->connect(boost::bind(&ModuleConnect::simpleReference, &mc, _1));
	bs1->connect(boost::bind(&ModuleConnect::simpleReferenceConst, &mc, _1));
	bs1->connect(boost::bind(&ModuleConnect::extendedReference, &mc, _1, _2));
	bs1->connect(boost::bind(&ModuleConnect::extendedReferenceConst, &mc, _1, _2));

	BFG::Event::Callable* cs2 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<std::string, BFG::GameHandle>* bs2 = static_cast<BFG::Event::Binding<std::string, BFG::GameHandle>*>(cs2);

	std::string s = "Hello";
	bs2->emit(s, senderHandle);
	s = "Du";
	bs2->emit(s, senderHandle);

	BFG::Event::Callable* cs3 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<std::string, BFG::GameHandle>* bs3 = static_cast<BFG::Event::Binding<std::string, BFG::GameHandle>*>(cs3);

	bs3->call();

	BFG::Event::Callable* cs4 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	delete cs4;
}

BOOST_AUTO_TEST_CASE (ConnectionTestVoid)
{
	ModuleConnect mc;
	BFG::u32 id = 18;
	BFG::GameHandle senderHandle = 225;
	BFG::GameHandle dest = BFG::NULL_HANDLE;

	BFG::Event::Callable* cv1 = new BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>();

	BFG::Event::Connection<BFG::u32, BFG::GameHandle> con1 = {id, dest, cv1};

	BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>* bv1 = static_cast<BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>*>(cv1);

	bv1->connect(boost::bind(&ModuleConnect::simpleVoid, &mc));
	bv1->connect(boost::bind(&ModuleConnect::simpleVoidConst, &mc));
	bv1->connect(boost::bind(&ModuleConnect::extendedVoid, &mc, _2));
	bv1->connect(boost::bind(&ModuleConnect::extendedVoidConst, &mc, _2));

	BFG::Event::Callable* cv2 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>* bv2 = static_cast<BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>*>(cv2);

	BFG::Event::Void v;
	bv2->emit(v, senderHandle);
	bv2->emit(v, senderHandle + 1);

	BFG::Event::Callable* cv3 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>* bv3 = static_cast<BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>*>(cv3);

	bv3->call();

	BFG::Event::Callable* cv4 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	delete cv4;
}

BOOST_AUTO_TEST_CASE (BinderTest)
{
	ModuleConnect mc;
	BFG::u32 idU = 92;
	BFG::u32 idS = 192;
	BFG::u32 idV = 292;
	BFG::GameHandle senderHandle = 8;
	BFG::GameHandle dest = BFG::NULL_HANDLE;

	BFG::Event::Binder<BFG::u32, BFG::GameHandle, BFG::GameHandle> b;

	b.connect<BFG::u32>(idU, boost::bind(&ModuleConnect::simpleCopy, &mc, _1), dest);
	b.connect<BFG::u32>(idU, boost::bind(&ModuleConnect::simpleCopyConst, &mc, _1), dest);
	b.connect<BFG::u32>(idU, boost::bind(&ModuleConnect::extendedCopy, &mc, _1, _2), dest);
	b.connect<BFG::u32>(idU, boost::bind(&ModuleConnect::extendedCopyConst, &mc, _1, _2), dest);

	b.connect<std::string>(idS, boost::bind(&ModuleConnect::simpleReference, &mc, _1), dest);
	b.connect<std::string>(idS, boost::bind(&ModuleConnect::simpleReferenceConst, &mc, _1), dest);
	b.connect<std::string>(idS, boost::bind(&ModuleConnect::extendedReference, &mc, _1, _2), dest);
	b.connect<std::string>(idS, boost::bind(&ModuleConnect::extendedReferenceConst, &mc, _1, _2), dest);

	b.connect<BFG::Event::Void>(idV, boost::bind(&ModuleConnect::simpleVoid, &mc), dest);
	b.connect<BFG::Event::Void>(idV, boost::bind(&ModuleConnect::simpleVoidConst, &mc), dest);
	b.connect<BFG::Event::Void>(idV, boost::bind(&ModuleConnect::extendedVoid, &mc, _2), dest);
	b.connect<BFG::Event::Void>(idV, boost::bind(&ModuleConnect::extendedVoidConst, &mc, _2), dest);

	BFG::u32 pu = 77;
	std::string ps = "StringPayload";
	BFG::Event::Void pv;

	b.emit(idU, pu, dest, senderHandle);
	b.emit(idS, ps, dest, senderHandle);
	b.emit(idV, pv, dest, senderHandle);

	b.tick();
}

BOOST_AUTO_TEST_CASE (ManualBind)
{
	boost::signals2::signal<void (const BFG::u32&, const BFG::GameHandle&)> sigU32;
	boost::signals2::signal<void (const std::string&, const BFG::GameHandle&)> sigString;
	boost::signals2::signal<void (const BFG::Event::Void&, const BFG::GameHandle&)> sigVoid;

	ModuleConnect mc;

 	sigU32.connect(boost::bind(&ModuleConnect::simpleCopy, &mc, _1));
	sigU32.connect(boost::bind(&ModuleConnect::simpleCopyConst, &mc, _1));
	sigU32.connect(boost::bind(&ModuleConnect::extendedCopy, &mc, _1, _2));
	sigU32.connect(boost::bind(&ModuleConnect::extendedCopyConst, &mc, _1, _2));

	sigString.connect(boost::bind(&ModuleConnect::simpleReference, &mc, _1));
	sigString.connect(boost::bind(&ModuleConnect::simpleReferenceConst, &mc, _1));
	sigString.connect(boost::bind(&ModuleConnect::extendedReference, &mc, _1, _2));
	sigString.connect(boost::bind(&ModuleConnect::extendedReferenceConst, &mc, _1, _2));

	sigVoid.connect(boost::bind(&ModuleConnect::simpleVoid, &mc));
	sigVoid.connect(boost::bind(&ModuleConnect::simpleVoidConst, &mc));
	sigVoid.connect(boost::bind(&ModuleConnect::extendedVoid, &mc, _2));
	sigVoid.connect(boost::bind(&ModuleConnect::extendedVoidConst, &mc, _2));

	sigU32(5, 22);
	sigString(std::string("Bla"), 15);
	sigVoid(BFG::Event::Void(), 48);
}


BOOST_AUTO_TEST_CASE (Connect)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	// EntryPoints
	lane.addEntry<ModuleConnect>();

	sync.startEntries();

	BFG::u32 u = 5;
	std::string s("Hallo");
	BFG::Event::Void v;
	lane.emit(1, u);
	lane.emit(2, u);
	lane.emit(3, u);
	lane.emit(4, u);
	lane.emit(5, s);
	lane.emit(6, s);
	lane.emit(7, s);
	lane.emit(8, s);
	lane.emit(9, v);
	lane.emit(10, v);
	lane.emit(11, v);
	lane.emit(12, v);
	sync.finish();
}

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

BOOST_AUTO_TEST_CASE (CopyParameter)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	lane.addEntry<TestModuleCopy>();
	gu32EventCounter = 0;

	sync.startEntries();

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

	sync.startEntries();

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

BOOST_AUTO_TEST_CASE (TwoLanesTwoEntryPointsTwoHandlerWithDifferentPayloadsForSameEvent)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane1(sync, 100);
	BFG::Event::Lane lane2(sync, 100);

	gf32EventCounter = 0;
	gu32EventCounter = 0;

	lane1.addEntry<TestModule>();
	lane2.addEntry<TestModule3>(); // shouldn't this cause an IncompatibleTypeException?

	sync.startEntries();

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

	sync.startEntries();

	BOOST_REQUIRE_THROW(lane.emit(1, std::string("Hello")), BFG::Event::IncompatibleTypeException);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (EntryWithStartParameter)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	BFG::u32 frequency = 100;
	lane.addEntry<TestModule4>(frequency);

	sync.startEntries();

	sync.finish();
}



#if 0
BOOST_AUTO_TEST_CASE (TwoConnectsSameEvent){}
BOOST_AUTO_TEST_CASE (TwoConnectsSameEventButDifferentPayloads){}
BOOST_AUTO_TEST_CASE (TwoLanesOneConnectPerLaneSameEvent){}
BOOST_AUTO_TEST_CASE (TwoLanesOneConnectPerLaneSameEventButDifferentPayload){}
BOOST_AUTO_TEST_CASE (OneLaneWithoutFinish){}
#endif



BOOST_AUTO_TEST_SUITE_END()
