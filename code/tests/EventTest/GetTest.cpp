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

#include <EventSystem/Event.h>

#include <boost/variant.hpp>
#include <boost/test/unit_test.hpp>

#include <Base/Logger.h>
#include <Core/GameHandle.h>

// ---------------------------------------------------------------------------

class Type1{};
class Type2{};

typedef boost::variant
<
	Type1,
	Type2
> TestPayloadT;

typedef BFG::Event
<
	EventIdT,
	TestPayloadT,
	BFG::GameHandle,
	BFG::GameHandle
> TestEvent;

BOOST_AUTO_TEST_SUITE(GetTestSuite)

BOOST_AUTO_TEST_CASE (TestFailingGet)
{
	// Create event containing Type2
	TestEvent e(123, Type2(), 456, 789);
	
	// Expect boost::get to fail with Type1
	BOOST_REQUIRE_THROW(boost::get<Type1>(e.data()), boost::bad_get);

	// Expect no exception with BFG::get for Type1
	BOOST_REQUIRE_THROW(BFG::get<Type1>(e), std::logic_error);
}

BOOST_AUTO_TEST_CASE (TestConstReferenceGet)
{
	// Create event containing Type2
	TestEvent e(1, Type1(), 2, 3);
	
	// Expect no exception with boost::get for Type1
	BOOST_REQUIRE_NO_THROW(boost::get<const Type1&>(e.data()));

	// Expect no exception with BFG::get for Type1
	BOOST_REQUIRE_NO_THROW(BFG::get<const Type1&>(e));
}

BOOST_AUTO_TEST_CASE (TestSuccessfulGet)
{
	// Create event containing Type1
	TestEvent e(111, Type1(), 222, 333);
	
	// Expect boost::get to fail with Type1
	BOOST_REQUIRE_NO_THROW(boost::get<Type1>(e.data()));

	// Expect no exception with BFG::get for Type1
	BOOST_REQUIRE_NO_THROW(BFG::get<Type1>(e));
}

BOOST_AUTO_TEST_CASE (DefaultHandleEventTest)
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG);
	
	TestEvent e(1, Type1(), 2, 3);
	TestEvent* ep = &e;
	
	DEFAULT_HANDLE_EVENT(&e);
	DEFAULT_HANDLE_EVENT(ep);
}

BOOST_AUTO_TEST_SUITE_END()
