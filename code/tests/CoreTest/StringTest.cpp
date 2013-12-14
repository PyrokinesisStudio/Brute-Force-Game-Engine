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


#include <Core/String.h>

#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(StringTest)

using namespace BFG;

BOOST_AUTO_TEST_CASE(testSplit)
{
    std::string token;
    std::string rest;
    std::string source = "first second third fourth";
    bool res = false;
    
    res = split(source, " ", token, rest);
	BOOST_CHECK(!token.empty());
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(token, "first");
}

BOOST_AUTO_TEST_CASE(testSplit2)
{
    std::string token;
    std::string rest;
    std::string source = "first-*-second-*-third-*-fourth";
    bool res = false;
    
    res = split(source, "-*-", token, rest);
	BOOST_CHECK(!token.empty());
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(token, "first");
}

BOOST_AUTO_TEST_CASE(testSplitNegative)
{
    std::string token;
    std::string rest;
    std::string source = "firstSecond";
    bool res = true;
    
	res = split(source, " ", token, rest);

    BOOST_CHECK(token.empty());
    BOOST_CHECK(rest.empty());
    BOOST_CHECK(!res);
}

BOOST_AUTO_TEST_CASE(testSplitAll)
{
    std::string source = "first second third fourth";
    std::vector<std::string> tokens;
    tokenize(source, " ", tokens);
    BOOST_CHECK(!tokens.empty());
	BOOST_CHECK_EQUAL(tokens.size(), 4);
    BOOST_CHECK_EQUAL(tokens[0], "first");
    BOOST_CHECK_EQUAL(tokens[1], "second");
    BOOST_CHECK_EQUAL(tokens[2], "third");
    BOOST_CHECK_EQUAL(tokens[3], "fourth");
}

BOOST_AUTO_TEST_CASE(tokenizeDequeTest)
{
    std::string source = "first second third fourth";
    std::deque<std::string> tokens;
    tokenize(source, " ", tokens);
    BOOST_CHECK(!tokens.empty());
	BOOST_CHECK_EQUAL(tokens.size(), 4);
    BOOST_CHECK_EQUAL(tokens[0], "first");
    BOOST_CHECK_EQUAL(tokens[1], "second");
    BOOST_CHECK_EQUAL(tokens[2], "third");
    BOOST_CHECK_EQUAL(tokens[3], "fourth");
}


BOOST_AUTO_TEST_CASE(testSplitAll2)
{
    std::string source = "first-*-second-*-third-*-fourth";
    std::vector<std::string> tokens;
    tokenize(source, "-*-", tokens);
    BOOST_CHECK(!tokens.empty());
	BOOST_CHECK_EQUAL(tokens.size(), 4);
    BOOST_CHECK_EQUAL(tokens[0], "first");
    BOOST_CHECK_EQUAL(tokens[1], "second");
    BOOST_CHECK_EQUAL(tokens[2], "third");
    BOOST_CHECK_EQUAL(tokens[3], "fourth");
}

BOOST_AUTO_TEST_SUITE_END()