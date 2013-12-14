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

#include <iostream>

#include <Core/Console.h>
#include <Core/String.h>

#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(ConsoleTest)

using namespace BFG;

u32 counterOne = 0;
u32 counterTwo = 0;
u32 counterThree = 0;
u32 counter3_1 = 0;
u32 counter3_2 = 0;
u32 counter4_1 = 0;

void resetCounter()
{
	counterOne = 0;
	counterTwo = 0;
	counterThree = 0;
	counter3_1 = 0;
	counter3_2 = 0;
	counter4_1 = 0;
}

void comOne(const std::string& comBody, Event::SubLanePtr)
{
	++counterOne;
	std::cout << "ONE " << comBody << "\n";
}

void comTwo(const std::string& comBody, Event::SubLanePtr)
{
	++counterTwo;
	std::cout << "Two " << comBody << "\n";
}

void comThree(const std::string& comBody, Event::SubLanePtr)
{
	++counterThree;
	std::cout << "Three " << comBody << "\n";
}

void com3_1(const std::string& comBody, Event::SubLanePtr)
{
	++counter3_1;
	std::cout << "3_1 " << comBody << "\n";
}

void com3_2(const std::string& comBody, Event::SubLanePtr)
{
	++counter3_2;
	std::cout << "3_2" << comBody << "\n";
}

void com4_1(const std::string& comBody, Event::SubLanePtr)
{
	++counter4_1;
	std::cout << "4_1" << comBody << "\n";
}

BOOST_AUTO_TEST_CASE(CommandTest1)
{
    std::string line = "one two three";
    std::deque<std::string> tokens;
	tokenize(line, " ", tokens);
	
	Command command;

	BOOST_REQUIRE_NO_THROW(command.add(tokens, &comOne));
	BOOST_CHECK_EQUAL(command.mCommands.size(), 1);
	
	BOOST_CHECK_NO_THROW
	(
		command.mCommands["one"]->
		mCommands["two"]->
		mCommands["three"]->
		mFunction("Hello world of Console.", Event::SubLanePtr())
	);
}

BOOST_AUTO_TEST_CASE(ConsoleTest)
{
	CommandPtrT commands(new Command);

	BOOST_REQUIRE_NO_THROW(registerCommand("one", &comOne, *commands));
	BOOST_REQUIRE_NO_THROW(registerCommand("two", &comTwo, *commands));
	BOOST_REQUIRE_NO_THROW(registerCommand("three", &comThree, *commands));
	BOOST_REQUIRE_NO_THROW(registerCommand("one two three1", &com3_1, *commands));
	BOOST_REQUIRE_NO_THROW(registerCommand("one two three2", &com3_2, *commands));
	BOOST_REQUIRE_NO_THROW(registerCommand("one two three four", &com4_1, *commands));

	BOOST_REQUIRE_EQUAL(commands->mCommands.size(), 3);
	BOOST_REQUIRE_EQUAL(commands->mCommands["one"]->mCommands.size(), 1);
	BOOST_REQUIRE_EQUAL(commands->mCommands["one"]->mCommands["two"]->mCommands.size(), 3);
	BOOST_REQUIRE_EQUAL(commands->mCommands["two"]->mCommands.size(), 0);

	Event::Synchronizer sync;
	Event::Lane lane(sync, 100);

	std::vector<s32> ev;
	ev.push_back(1);
	ev.push_back(3);

	Console console(commands, ev, lane.createSubLane());

	resetCounter();

	typedef std::string s;

	lane.emit(1, s("one slfdjls"));
	lane.emit(1, s("one slfggggggggdfdfg"));
	lane.emit(2, s("one slfggggggggdfdfg")); // negative wrong event id
	lane.emit(3, s("one slfggggggggdfdfg"));

	lane.emit(3, s("two slfggggggggdfdfg"));
	lane.emit(1, s("two slfggggggggdfdfg"));

	lane.emit(1, s("three slfggggggggdfdfg"));

	lane.emit(1, s("one two three1 slfggggggggdfdfg"));
	lane.emit(1, s("one two three1 slfggggggggdfdfg"));

	lane.emit(1, s("one two three2 slfggggggggdfdfg"));
	lane.emit(3, s("one two three2 slfggggggggdfdfg"));
	lane.emit(1, s("one two three2 slfggggggggdfdfg"));

	lane.emit(3, s("one two three four slfggggggggdfdfg"));
	// no command body
	lane.emit(1, s("one two three four"));

	// negatives
	lane.emit(3, s("onedsf dgfdg")); 
	lane.emit(3, s("one two slfggggggggdfdfg"));
	lane.emit(3, s("one two three four1 sdffds"));
	
	sync.start();
	sync.finish();

	BOOST_CHECK_EQUAL(counterOne, 3);
	BOOST_CHECK_EQUAL(counterTwo, 2);
	BOOST_CHECK_EQUAL(counterThree, 1);
	
	BOOST_CHECK_EQUAL(counter3_1, 2);
	BOOST_CHECK_EQUAL(counter3_2, 3);

	BOOST_CHECK_EQUAL(counter4_1, 2);
}

BOOST_AUTO_TEST_SUITE_END()