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

//! Handy shortcut for BOOST_CHECK_EQUAL_COLLECTIONS
#define CHECK_EQUAL_COLLECTIONS(a,b) \
	BOOST_CHECK_EQUAL_COLLECTIONS((a).begin(), (a).end(), (b).begin(), (b).end())

//! Helper module for automatic unit tests.
struct Case
{
	BFG::u32 mCalls;
	std::vector<std::string> mStrings;
	std::vector<BFG::u32> mU32;
	std::vector<BFG::Event::SenderT> mSenders;
};

typedef std::map<std::string, Case> EventCounter;

//! Helper module for automatic unit tests.
struct ModuleConnect : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	ModuleConnect(EventCounter& ec) :
	mEventCounter(ec)
	{}
	
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

	void simpleCopy(BFG::u32 u)
	{
		mEventCounter["simpleCopy"].mCalls++;
		mEventCounter["simpleCopy"].mU32.push_back(u);
	}

	void simpleCopyConst(BFG::u32 u) const
	{
		mEventCounter["simpleCopyConst"].mCalls++;
		mEventCounter["simpleCopyConst"].mU32.push_back(u);
	}

	void extendedCopy(BFG::u32 u, const BFG::GameHandle& sender)
	{
		mEventCounter["extendedCopy"].mCalls++;
		mEventCounter["extendedCopy"].mU32.push_back(u);
		mEventCounter["extendedCopy"].mSenders.push_back(sender);
	}

	void extendedCopyConst(BFG::u32 u, const BFG::GameHandle& sender) const 
	{
		mEventCounter["extendedCopyConst"].mCalls++;
		mEventCounter["extendedCopyConst"].mU32.push_back(u);
		mEventCounter["extendedCopyConst"].mSenders.push_back(sender);
	}

	void simpleReference(const std::string& s)
	{
		mEventCounter["simpleReference"].mCalls++;
		mEventCounter["simpleReference"].mStrings.push_back(s);
	}

	void simpleReferenceConst(const std::string& s) const 
	{
		mEventCounter["simpleReferenceConst"].mCalls++;
		mEventCounter["simpleReferenceConst"].mStrings.push_back(s);
	}

	void extendedReference(const std::string& s, const BFG::GameHandle& sender)
	{
		mEventCounter["extendedReference"].mCalls++;
		mEventCounter["extendedReference"].mStrings.push_back(s);
		mEventCounter["extendedReference"].mSenders.push_back(sender);
	}

	void extendedReferenceConst(const std::string& s, const BFG::GameHandle& sender) const 
	{
		mEventCounter["extendedReferenceConst"].mCalls++;
		mEventCounter["extendedReferenceConst"].mStrings.push_back(s);
		mEventCounter["extendedReferenceConst"].mSenders.push_back(sender);
	}

	void simpleVoid()
	{
		mEventCounter["simpleVoid"].mCalls++;
	}

	void simpleVoidConst() const 
	{
		mEventCounter["simpleVoidConst"].mCalls++;
	}

	void extendedVoid(const BFG::GameHandle& sender)
	{
		mEventCounter["extendedVoid"].mCalls++;
		mEventCounter["extendedVoid"].mSenders.push_back(sender);
	}

	void extendedVoidConst(const BFG::GameHandle& sender) const 
	{
		mEventCounter["extendedVoidConst"].mCalls++;
		mEventCounter["extendedVoidConst"].mSenders.push_back(sender);
	}

	EventCounter& mEventCounter;
};

//! Helper module for automatic unit tests.
struct SubModuleConnect : ModuleConnect
{
	SubModuleConnect(EventCounter& ec) :
	ModuleConnect(ec)
	{}
	
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
		std::cout << "SubModuleConnect.onDestroy -> sublane.reset" << std::endl;
		sublane.reset();
	}
	
	BFG::Event::Lane* lane;
	boost::shared_ptr<BFG::Event::SubLane> sublane;
};

// ---------------------------------------------------------------------------


BOOST_AUTO_TEST_SUITE(TestSuite)

//! Testing basic SubLane functionality: Emits, Proper Destruction etc.
BOOST_AUTO_TEST_CASE (SubLaneTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane* lane_ptr = new BFG::Event::Lane(sync, 100);
	BFG::Event::Lane& lane = *lane_ptr;
	boost::shared_ptr<BFG::Event::SubLane> sublane(lane.createSubLane());
	
	// EntryPoints
	EventCounter ec;
	lane.addEntry<SubModuleConnect>(ec);
	sync.startEntries();
	
	sublane->emit(5, std::string("SubLane to SubLane"));
	sublane->emit(6, std::string("SubLane to SubLane (again)"));
	lane.emit(7, std::string("Lane to SubLane"));
	lane.emit(13, BFG::Event::Void());
	
	boost::this_thread::sleep(boost::posix_time::millisec(100));
	
	lane.emit(5, std::string("Should not be emitted"));
	
	sync.finish();

	delete lane_ptr;

	sublane->emit(5, std::string("Sublane after finish"));
}

//! Testing basic Binding functionality.
BOOST_AUTO_TEST_CASE (BindingTest)
{
	EventCounter ec;
	ModuleConnect mc(ec);
	BFG::GameHandle senderHandle = 83;
	
	// Copy
	BFG::Event::Binding<BFG::u32, BFG::GameHandle> bu;

	bu.connect(boost::bind(&ModuleConnect::simpleCopy, &mc, _1));
	bu.connect(boost::bind(&ModuleConnect::simpleCopyConst, &mc, _1));
	bu.connect(boost::bind(&ModuleConnect::extendedCopy, &mc, _1, _2));
	bu.connect(boost::bind(&ModuleConnect::extendedCopyConst, &mc, _1, _2));

	BFG::u32 u = 15;
	bu.emit(u, senderHandle);

	bu.call();
	
	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mCalls, 1);
	
	// Correct values?
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mU32[0], u);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mU32[0], u);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mU32[0], u);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mU32[0], u);

	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mSenders[0], senderHandle);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mSenders[0], senderHandle);
	
	// Reference
	BFG::Event::Binding<std::string, BFG::GameHandle> bs;

	bs.connect(boost::bind(&ModuleConnect::simpleReference, &mc, _1));
	bs.connect(boost::bind(&ModuleConnect::simpleReferenceConst, &mc, _1));
	bs.connect(boost::bind(&ModuleConnect::extendedReference, &mc, _1, _2));
	bs.connect(boost::bind(&ModuleConnect::extendedReferenceConst, &mc, _1, _2));

	std::string s = "Hello";
	bs.emit(s, senderHandle);

	bs.call();

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleReference"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mCalls, 1);
	
	// Correct values?
	BOOST_CHECK_EQUAL(ec["simpleReference"].mStrings[0], s);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mStrings[0], s);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mStrings[0], s);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mStrings[0], s);

	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedReference"].mSenders[0], senderHandle);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mSenders[0], senderHandle);
	
	// Void
	BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle> bv;

	bv.connect(boost::bind(&ModuleConnect::simpleVoid, &mc));
	bv.connect(boost::bind(&ModuleConnect::simpleVoidConst, &mc));
	bv.connect(boost::bind(&ModuleConnect::extendedVoid, &mc, _2));
	bv.connect(boost::bind(&ModuleConnect::extendedVoidConst, &mc, _2));

	BFG::Event::Void v;
	bv.emit(v, senderHandle);

	bv.call();
	
	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleVoid"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleVoidConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mCalls, 1);
	
	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mSenders[0], senderHandle);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mSenders[0], senderHandle);
}

BOOST_AUTO_TEST_CASE (ConnectionTestU32)
{
	EventCounter ec;
	ModuleConnect mc(ec);

	typedef BFG::Event::Binding<BFG::u32, BFG::GameHandle> BindingT;

	BFG::u32 id = 32;
	BFG::GameHandle dest = BFG::generateHandle();

	// Create a new Binding and try to put it into a Connection object,
	// then try to get it back.
	BFG::Event::Callable* cu1 = new BindingT();
	BFG::Event::Connection<BFG::u32, BFG::GameHandle> con1 = {id, dest, cu1};
	BindingT* bu1 = static_cast<BindingT*>(cu1);
	
	BOOST_CHECK_EQUAL(con1.mEventId, id);
	BOOST_CHECK_EQUAL(con1.mDestinationId, dest);
	
	// Use the Binding for connects
	bu1->connect(boost::bind(&ModuleConnect::simpleCopy, &mc, _1));
	bu1->connect(boost::bind(&ModuleConnect::simpleCopyConst, &mc, _1));
	bu1->connect(boost::bind(&ModuleConnect::extendedCopy, &mc, _1, _2));
	bu1->connect(boost::bind(&ModuleConnect::extendedCopyConst, &mc, _1, _2));

	// Again, get the Binding from the Connection
	BFG::Event::Callable* cu2 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BindingT* bu2 = static_cast<BindingT*>(cu2);

	// Emit some data on it
	std::vector<BFG::u32> testData;
	testData.push_back(55);
	testData.push_back(43);
	
	std::vector<BFG::GameHandle> testSender;
	testSender.push_back(123);
	testSender.push_back(456);
	
	bu2->emit(testData[0], testSender[0]);
	bu2->emit(testData[1], testSender[1]);

	// Again, get the Binding from the Connection
	BFG::Event::Callable* cu3 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BindingT* bu3 = static_cast<BindingT*>(cu3);

	// Perform event delivery
	bu3->call();

	// Due to 2 emits we expect 2 calls.
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mCalls, 2);
	
	// Correct values?
	CHECK_EQUAL_COLLECTIONS(ec["simpleCopy"].mU32, testData);
	CHECK_EQUAL_COLLECTIONS(ec["simpleCopyConst"].mU32, testData);
	CHECK_EQUAL_COLLECTIONS(ec["extendedCopy"].mU32, testData);
	CHECK_EQUAL_COLLECTIONS(ec["extendedCopyConst"].mU32, testData);
	
	// Sender correct?
	CHECK_EQUAL_COLLECTIONS(ec["extendedCopy"].mSenders, testSender);
	CHECK_EQUAL_COLLECTIONS(ec["extendedCopyConst"].mSenders, testSender);
	
	// Cleanup
	BFG::Event::Callable* cu4 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	delete cu4;
}

BOOST_AUTO_TEST_CASE (ConnectionTestString)
{
	EventCounter ec;
	ModuleConnect mc(ec);
	
	typedef BFG::Event::Binding<std::string , BFG::GameHandle> BindingT;
	
	BFG::u32 id = 18;
	BFG::GameHandle dest = BFG::generateHandle();

	// Create a new Binding and try to put it into a Connection object,
	// then try to get it back.
	BFG::Event::Callable* cs1 = new BindingT();
	BFG::Event::Connection<BFG::u32, BFG::GameHandle> con1 = {id, dest, cs1};
	BindingT* bs1 = static_cast<BindingT*>(cs1);

	BOOST_CHECK_EQUAL(con1.mEventId, id);
	BOOST_CHECK_EQUAL(con1.mDestinationId, dest);
	
	// Use the Binding for connects
	bs1->connect(boost::bind(&ModuleConnect::simpleReference, &mc, _1));
	bs1->connect(boost::bind(&ModuleConnect::simpleReferenceConst, &mc, _1));
	bs1->connect(boost::bind(&ModuleConnect::extendedReference, &mc, _1, _2));
	bs1->connect(boost::bind(&ModuleConnect::extendedReferenceConst, &mc, _1, _2));

	// Again, get the Binding from the Connection
	BFG::Event::Callable* cs2 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BindingT* bs2 = static_cast<BindingT*>(cs2);
	
	// Emit some data on it
	std::vector<std::string> testData;
	testData.push_back("Hello");
	testData.push_back("Du");
	
	std::vector<BFG::GameHandle> testSender;
	testSender.push_back(123);
	testSender.push_back(456);
	
	bs2->emit(testData[0], testSender[0]);
	bs2->emit(testData[1], testSender[1]);

	// Again, get the Binding from the Connection
	BFG::Event::Callable* cs3 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BindingT* bs3 = static_cast<BindingT*>(cs3);

	// Perform event delivery
	bs3->call();

	// Due to 2 emits we expect 2 calls.
	BOOST_CHECK_EQUAL(ec["simpleReference"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mCalls, 2);
	
	// Correct values?
	CHECK_EQUAL_COLLECTIONS(ec["simpleReference"].mStrings, testData);
	CHECK_EQUAL_COLLECTIONS(ec["simpleReferenceConst"].mStrings, testData);
	CHECK_EQUAL_COLLECTIONS(ec["extendedReference"].mStrings, testData);
	CHECK_EQUAL_COLLECTIONS(ec["extendedReferenceConst"].mStrings, testData);
	
	// Sender correct?
	CHECK_EQUAL_COLLECTIONS(ec["extendedReference"].mSenders, testSender);
	CHECK_EQUAL_COLLECTIONS(ec["extendedReferenceConst"].mSenders, testSender);
	
	// Cleanup
	BFG::Event::Callable* cs4 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	delete cs4;
}

BOOST_AUTO_TEST_CASE (ConnectionTestVoid)
{
	EventCounter ec;
	ModuleConnect mc(ec);
	
	typedef BFG::Event::Binding<BFG::Event::Void , BFG::GameHandle> BindingT;
	
	BFG::u32 id = 18;
	BFG::GameHandle dest = BFG::generateHandle();

	// Create a new Binding and try to put it into a Connection object,
	// then try to get it back.
	BFG::Event::Callable* cv1 = new BindingT();
	BFG::Event::Connection<BFG::u32, BFG::GameHandle> con1 = {id, dest, cv1};
	BindingT* bv1 = static_cast<BindingT*>(cv1);

	// Use the Binding for connects
	bv1->connect(boost::bind(&ModuleConnect::simpleVoid, &mc));
	bv1->connect(boost::bind(&ModuleConnect::simpleVoidConst, &mc));
	bv1->connect(boost::bind(&ModuleConnect::extendedVoid, &mc, _2));
	bv1->connect(boost::bind(&ModuleConnect::extendedVoidConst, &mc, _2));

	// Again, get the Binding from the Connection
	BFG::Event::Callable* cv2 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BindingT* bv2 = static_cast<BindingT*>(cv2);

	std::vector<BFG::GameHandle> testSender;
	testSender.push_back(123);
	testSender.push_back(456);
	
	BFG::Event::Void v;
	bv2->emit(v, testSender[0]);
	bv2->emit(v, testSender[1]);

	BFG::Event::Callable* cv3 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>* bv3 = static_cast<BFG::Event::Binding<BFG::Event::Void, BFG::GameHandle>*>(cv3);

	bv3->call();

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleVoid"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["simpleVoidConst"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mCalls, 2);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mCalls, 2);
	
	// Sender correct?
	CHECK_EQUAL_COLLECTIONS(ec["extendedVoid"].mSenders, testSender);
	CHECK_EQUAL_COLLECTIONS(ec["extendedVoidConst"].mSenders, testSender);
	
	BFG::Event::Callable* cv4 = boost::any_cast<BFG::Event::Callable*>(con1.mBinding);
	delete cv4;
}

BOOST_AUTO_TEST_CASE (BinderTest)
{
	EventCounter ec;
	ModuleConnect mc(ec);
	BFG::u32 idU = 92;
	BFG::u32 idS = 192;
	BFG::u32 idV = 292;
	
	BFG::GameHandle senderHandle = 8;
	BFG::GameHandle dest = BFG::generateHandle();

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

	BFG::u32 pu1 = 77;
	BFG::u32 pu2 = 88;
	std::string ps1 = "StringPayload1";
	std::string ps2 = "StringPayload2";
	BFG::Event::Void pv;

	b.emit(idU, pu1, BFG::NULL_HANDLE, senderHandle); // this should not be received
	b.emit(idU, pu2, dest, senderHandle);
	b.emit(idS, ps1, BFG::NULL_HANDLE, senderHandle); // this should not be received
	b.emit(idS, ps2, dest, senderHandle);
	b.emit(idV, pv, BFG::NULL_HANDLE, senderHandle); // this should not be received
	b.emit(idV, pv, dest, senderHandle);

	b.tick();

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mCalls, 1);
	
	// Correct values?
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mU32[0], pu2);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mU32[0], pu2);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mU32[0], pu2);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mU32[0], pu2);

	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mSenders[0], senderHandle);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mSenders[0], senderHandle);

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleReference"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mCalls, 1);
	
	// Correct values?
	BOOST_CHECK_EQUAL(ec["simpleReference"].mStrings[0], ps2);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mStrings[0], ps2);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mStrings[0], ps2);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mStrings[0], ps2);

	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedReference"].mSenders[0], senderHandle);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mSenders[0], senderHandle);

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleVoid"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleVoidConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mCalls, 1);
	
	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mSenders[0], senderHandle);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mSenders[0], senderHandle);
}

BOOST_AUTO_TEST_CASE (Connect)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);

	// EntryPoints
	EventCounter ec;
	lane.addEntry<ModuleConnect>(ec);

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

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mCalls, 1);
	
	// Correct values?
	BOOST_CHECK_EQUAL(ec["simpleCopy"].mU32[0], u);
	BOOST_CHECK_EQUAL(ec["simpleCopyConst"].mU32[0], u);
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mU32[0], u);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mU32[0], u);

	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedCopy"].mSenders[0], BFG::NULL_HANDLE);
	BOOST_CHECK_EQUAL(ec["extendedCopyConst"].mSenders[0], BFG::NULL_HANDLE);

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleReference"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mCalls, 1);
	
	// Correct values?
	BOOST_CHECK_EQUAL(ec["simpleReference"].mStrings[0], s);
	BOOST_CHECK_EQUAL(ec["simpleReferenceConst"].mStrings[0], s);
	BOOST_CHECK_EQUAL(ec["extendedReference"].mStrings[0], s);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mStrings[0], s);

	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedReference"].mSenders[0], BFG::NULL_HANDLE);
	BOOST_CHECK_EQUAL(ec["extendedReferenceConst"].mSenders[0], BFG::NULL_HANDLE);

	// Correctly called?
	BOOST_CHECK_EQUAL(ec["simpleVoid"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["simpleVoidConst"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mCalls, 1);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mCalls, 1);
	
	// Sender correct?
	BOOST_CHECK_EQUAL(ec["extendedVoid"].mSenders[0], BFG::NULL_HANDLE);
	BOOST_CHECK_EQUAL(ec["extendedVoidConst"].mSenders[0], BFG::NULL_HANDLE);
}

BOOST_AUTO_TEST_SUITE_END()
