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

#include <string>

#include <boost/test/unit_test.hpp>

#include <Model/Data/Connection.h>
#include <Model/Property/Value.h>

#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-comparison"
#endif

BOOST_AUTO_TEST_SUITE(DataTestSuite)

BOOST_AUTO_TEST_CASE (ConnectionTest)
{
	BFG::Connection c;

	// these are not exported into the dll
	//BOOST_CHECK_EQUAL(c.good(), false);
	//BOOST_CHECK_EQUAL(c.str(), std::string("0@::0"));

	std::string testConnectionString("7@TestObject:TestModule:2");

	BOOST_CHECK_NO_THROW(BFG::parseConnection(testConnectionString, c));

	BOOST_CHECK_EQUAL(c.mConnectedLocalAt, 7);
	BOOST_CHECK_EQUAL(c.mConnectedExternToGameObject, std::string("TestObject"));
	BOOST_CHECK_EQUAL(c.mConnectedExternToModule, std::string("TestModule"));
	BOOST_CHECK_EQUAL(c.mConnectedExternAt, 2);

// 	BOOST_CHECK_EQUAL(c.str(), testConnectionString);
// 	BOOST_CHECK_EQUAL(c.good(), true);

	c.mConnectedLocalAt = 0;
	c.mConnectedExternToGameObject = "";
	c.mConnectedExternToModule = "";
	c.mConnectedExternAt = 0;

	std::string falseConnection("zzz@33aad:false");
	BOOST_CHECK_THROW(BFG::parseConnection(falseConnection, c), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (ValueTest)
{
	BFG::Property::Value v1;

	std::string testString("test");
	const BFG::u32 constU32(8);
	BFG::u32 testU32(12);
	BFG::u32& u32Ref(testU32);
	const BFG::u32& constU32Ref = constU32;
	BFG::f32 testF32(8.1f);

	// replacing types
	BOOST_CHECK_NO_THROW(v1 = testString);
	BOOST_CHECK_NO_THROW(v1 = constU32);
	BOOST_CHECK_NO_THROW(v1 = testU32);
	BOOST_CHECK_NO_THROW(v1 = constU32Ref);
	BOOST_CHECK_NO_THROW(v1 = 13);
	BOOST_CHECK_NO_THROW(v1 = u32Ref);

	BFG::Property::Value v2(v1);

	// can test same types even if const and/or ref
	BOOST_CHECK_NO_THROW(v2 == u32Ref);
	BOOST_CHECK_NO_THROW(v2 == 12u);
	BOOST_CHECK_NO_THROW(v2 == constU32Ref);
	BOOST_CHECK_NO_THROW(v2 == testU32);
	BOOST_CHECK_NO_THROW(v2 == constU32);
	BOOST_CHECK_THROW(v2 == testString, boost::bad_any_cast);

	BOOST_CHECK_NO_THROW(v1 = testString);

	// v2 is no reference of v1
	BOOST_CHECK_NO_THROW(v1 == testString);
	BOOST_CHECK_THROW(v2 == testString, boost::bad_any_cast);

	BOOST_CHECK_NO_THROW(v1 = testU32);

	// can not test even slightly different types
	BOOST_CHECK_THROW(v1 > 13, boost::bad_any_cast);
	BOOST_CHECK_THROW(v1 > testF32, boost::bad_any_cast);

	// can multiply with same type
	BFG::u32 u;
	BOOST_CHECK_NO_THROW(u = v1 * 20u);
	BOOST_CHECK_EQUAL(u, 240);


	// can ref cast (explicit/implicit)
	BOOST_CHECK_NO_THROW(u32Ref = (BFG::u32&)v1);
	BOOST_CHECK_NO_THROW(BFG::u32& r = v1);

	// ref check
	BFG::u32& ref = v1;
	ref = 6475u;
	bool refCheck = (v1 == 6475u);
	BOOST_CHECK_EQUAL(refCheck, true);

	// can const ref cast (explicit/implicit)
	BOOST_CHECK_NO_THROW(u32Ref = (const BFG::u32&)v1);
	BOOST_CHECK_NO_THROW(const BFG::u32& ref = v1);
}

BOOST_AUTO_TEST_SUITE_END()

#ifdef __clang__
	#pragma clang diagnostic pop
#endif
