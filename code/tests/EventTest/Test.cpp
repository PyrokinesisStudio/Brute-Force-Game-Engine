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
const BFG::GameHandle testDestinationId = 1234;
const BFG::GameHandle testSenderId = 5678;

bool o1 = false;
bool o2 = false;

// ---------------------------------------------------------------------------
struct HelloWorld
{
	HelloWorld(BFG::Event::Lane* lane, int id) :
	c1(0),
	c2(0),
	c3(0),
	c4(0),
	mLane(lane),
	mId(id)
	{}
	
	~HelloWorld()
	{
		std::cout << std::dec << "Received " << c1+c2+c3+c4 << " events.\n";
	}
	
	void other1(const std::string& s);
	
	void other2(const std::string& s)
	{
		std::cout << "physics (" << c2 << "), " << s << std::endl;
		++c2;
	}
	void other3(const std::string& s)
	{
		std::cout << "view (" << c3 << "), " << s << std::endl;
		++c3;
	}
	void other4(const std::string& s)
	{
		std::cout << "game (" << c4 << "), " << s << std::endl;
		++c4;
	}
	
	void update(const BFG::Event::TickData ld)
	{
		std::cout << "++++++++++update+++++++++++(" << ld.mTimeSinceLastTick << ")" << std::endl;
	}

	int c1,c2,c3,c4,mId;
	BFG::Event::Lane* mLane;
};

void HelloWorld::other1(const std::string& s)
{
	std::cout << "audio[" << mId << "]" << "(" << c1 << "), " << s << std::endl;
	++c1;
	boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	std::cout << "audio slept 300ms" << std::endl;
	mLane->emit(2, std::string("To View from Audio"));
}

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

	// Connect a HelloWorld slot
	HelloWorld hello(&audio, 15);
	HelloWorld hello2(&audio, 16);
	
	audio.connectLoop(boost::bind(&HelloWorld::update, boost::ref(hello), _1));
	audio.connect<std::string>(1, boost::bind(&HelloWorld::other1, boost::ref(hello), _1), 15);
	audio.connect<std::string>(1, boost::bind(&HelloWorld::other1, boost::ref(hello2), _1));
	physics.connect<std::string>(1, boost::bind(&HelloWorld::other2, boost::ref(hello), _1));
	view.connect<std::string>(2, boost::bind(&HelloWorld::other3, boost::ref(hello), _1));
	game.connect<std::string>(2, boost::bind(&HelloWorld::other4, boost::ref(hello), _1));
	
	//audio.connect<std::string>(1, &HelloWorld::other1, hello);
	for (int i=0; i<2; ++i)
	{
		physics.emit(1, std::string("Physics Boom"));
		audio.emit(2, std::string("Audio Boom"));
		view.emit(2, std::string("View Boom"));
		game.emit(1, std::string("Game Boom"), 15);
		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	}
	physics.emit(1, std::string("Ende"));
	sync.finish();
}

BOOST_AUTO_TEST_SUITE_END()
