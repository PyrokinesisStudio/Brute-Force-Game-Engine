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

#include <EventSystem/Emitter.h>

#include <boost/test/unit_test.hpp>

// Mocks
// #####

EventLoop* const loopMock = reinterpret_cast<EventLoop*>(0x1234);
EventLoop* const loopMock2 = reinterpret_cast<EventLoop*>(0x5678);

struct Derived : public BFG::Emitter
{
	explicit Derived(EventLoop* loop) :
	BFG::Emitter(loop)
	{}

	Derived(const Derived& other) :
	BFG::Emitter(other)
	{}
	
	Derived& operator = (const Derived& rhs)
	{ 
		BFG::Emitter::operator=(rhs);
		return *this;
	}
};

// Unit Tests
// ###########

BOOST_AUTO_TEST_SUITE(EmitterTestSuite)

BOOST_AUTO_TEST_CASE (testCtor)
{
	BFG::Emitter e(loopMock);

	BOOST_REQUIRE_EQUAL(e.loop(), loopMock);
}

BOOST_AUTO_TEST_CASE (testCopyCtor)
{
	BFG::Emitter a(loopMock);
	BFG::Emitter b(a);
	
	BOOST_REQUIRE_EQUAL(a.loop(), loopMock);
	BOOST_REQUIRE_EQUAL(a.loop(), b.loop());
}

BOOST_AUTO_TEST_CASE (testAssignment)
{
	BFG::Emitter a(loopMock);
	BFG::Emitter b(loopMock2);
	b = a;
	
	BOOST_REQUIRE_EQUAL(a.loop(), loopMock);
	BOOST_REQUIRE_EQUAL(a.loop(), b.loop());
}

BOOST_AUTO_TEST_CASE (testInheritance)
{
	BFG::Emitter a(loopMock);
	
	Derived d1(loopMock);     // Derived()
	Derived d2(d1);           // Derived(const Derived&)
	Derived d3(loopMock2);    // Derived()
	d3 = d2;                  // Derived& operator = (const Derived&)

	BOOST_REQUIRE_EQUAL(a.loop(), loopMock);
	BOOST_REQUIRE_EQUAL(a.loop(), d1.loop());
	BOOST_REQUIRE_EQUAL(a.loop(), d2.loop());
	BOOST_REQUIRE_EQUAL(a.loop(), d3.loop());
}

BOOST_AUTO_TEST_SUITE_END()
