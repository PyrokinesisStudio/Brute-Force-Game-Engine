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

const int testEventId = 5;
const BFG::GameHandle testDestinationId = 15;
const BFG::GameHandle testDestinationId2 = 16;
const BFG::GameHandle testSenderId = 5678;
const BFG::GameHandle testSenderId2 = 91011;

struct HelloWorld
{
	HelloWorld(BFG::Event::Lane* lane, int id) :
	c1(0),
	c2(0),
	c3(0),
	c4(0),
	mId(id),
	mLane(lane)
	{}
	
	~HelloWorld()
	{
		std::cout << std::dec << "Received " << c1+c2+c3+c4 << " events.\n";
	}
	
	void other1(const std::string& s, const BFG::GameHandle& sender);
	
	void other2(const std::string& s, const BFG::GameHandle& sender)
	{
		std::cout << "physics (#" << c2 << ") from (" << BFG::stringify(sender) << ") , " << s << std::endl;
		++c2;
	}
	void other3(const std::string& s, const BFG::GameHandle& sender)
	{
		std::cout << "view (#" << c3 << ") from (" << BFG::stringify(sender) << ") , " << s << std::endl;
		++c3;
	}
	void other4(const std::string& s, const BFG::GameHandle& sender)
	{
		std::cout << "game (#" << c4 << ") from (" << BFG::stringify(sender) << "), " << s << std::endl;
		++c4;
	}
	
	void update(const BFG::Event::TickData ld)
	{
		std::cout << "++++++++++update+++++++++++(" << ld.mTimeSinceLastTick << ")" << std::endl;
	}

	int c1,c2,c3,c4,mId;
	BFG::Event::Lane* mLane;
};

void HelloWorld::other1(const std::string& s, const BFG::GameHandle& sender)
{
	std::cout << "audio[" << mId << "]" << "(#" << c1 << ") from (" << BFG::stringify(sender) << ") , " << s << std::endl;
	++c1;
	boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	std::cout << "audio slept 300ms" << std::endl;
	mLane->emit(2, std::string("To View from Audio"));
}


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

	void event2Handler(const std::string& s, const BFG::GameHandle& sender)
	{
		std::cout << "TestModulView1: from (" << BFG::stringify(sender) << ") , " << s << std::endl;
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

	sync.startEntries();

	// Connect a HelloWorld slot
// 	HelloWorld hello(&audio, testDestinationId);
// 	HelloWorld hello2(&audio, testDestinationId2);
// 	
// 	audio.connectLoop(&hello, &HelloWorld::update);
// 	audio.connect(1, &hello, &HelloWorld::other1, testDestinationId);
//  	audio.connect(1, &hello2, &HelloWorld::other1);
//  	physics.connect(1, &hello, &HelloWorld::other2);
//  	view.connect(2, &hello, &HelloWorld::other3);
//  	game.connect(2, &hello, &HelloWorld::other4);
// 	
// 	//audio.connect<std::string>(1, &HelloWorld::other1, hello);
// 	for (int i=0; i<2; ++i)
// 	{
// 		physics.emit(1, 5); // provoking an IncompatibleTypeException
// 		audio.emit(2, std::string("Audio Boom"), 0, testSenderId2);
// 		view.emit(2, std::string("View Boom"), 0, testSenderId);
// 		game.emit(1, std::string("Game Boom"), testDestinationId);
// 		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
// 	}
// 	physics.emit(1, std::string("Ende"));
	sync.finish();
}

BOOST_AUTO_TEST_SUITE_END()
