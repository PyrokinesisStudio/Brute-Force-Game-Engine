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

#include <Model/LevelOfDetail.h>

#include <boost/test/unit_test.hpp>

#include "Utils.h"

BOOST_AUTO_TEST_SUITE(LoDTest)

BOOST_AUTO_TEST_CASE (QuantifierTest)
{
	using namespace BFG;

	BOOST_CHECK_THROW(Quantifier q1(0.3f, 0.4f, 0.4f, 0.3f, 0.3f, 0.4f), std::logic_error);
	BOOST_CHECK_THROW(Quantifier q2(0.3f, 0.3f, 0.4f, 0.4f, 0.3f, 0.4f), std::logic_error);
	BOOST_CHECK_THROW(Quantifier q3(0.4f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f), std::logic_error);

	BOOST_CHECK_NO_THROW(Quantifier q4(0.4f, 0.3f, 0.3f, 0.5f, 0.35f, 0.15f));
}

BOOST_AUTO_TEST_CASE (LevelOfDetailTest)
{
	using namespace BFG;

	Quantifier q(0.4f, 0.3f, 0.3f, 0.5f, 0.35f, 0.15f);

	Ranges::RangeTableT dtC;
	dtC.push_back(Ranges::RangeDescriptorT(1, 0.0f));
	dtC.push_back(Ranges::RangeDescriptorT(2, 10.0f));
	dtC.push_back(Ranges::RangeDescriptorT(4, 50.0f));
	dtC.push_back(Ranges::RangeDescriptorT(10, 200.0f));

	Ranges::RangeTableT soO;
	soO.push_back(Ranges::RangeDescriptorT(1, 200.0f));
	soO.push_back(Ranges::RangeDescriptorT(2, 100.0f));
	soO.push_back(Ranges::RangeDescriptorT(4, 50.0f));
	soO.push_back(Ranges::RangeDescriptorT(6, 30.0f));
	soO.push_back(Ranges::RangeDescriptorT(8, 10.0f));
	soO.push_back(Ranges::RangeDescriptorT(10, 0.0f));

	Ranges::RangeTableT vel;
	vel.push_back(Ranges::RangeDescriptorT(1, 300.0f));
	vel.push_back(Ranges::RangeDescriptorT(2, 200.0f));
	vel.push_back(Ranges::RangeDescriptorT(3, 100.0f));
	vel.push_back(Ranges::RangeDescriptorT(5, 50.0f));
	vel.push_back(Ranges::RangeDescriptorT(8, 10.0f));
	vel.push_back(Ranges::RangeDescriptorT(10, 0.0f));

	Ranges::RangeTableT direc;
	direc.push_back(Ranges::RangeDescriptorT(1, 0.0f));
	direc.push_back(Ranges::RangeDescriptorT(2, 10.0f));
	direc.push_back(Ranges::RangeDescriptorT(3, 30.0f));
	direc.push_back(Ranges::RangeDescriptorT(6, 90.0f));
	direc.push_back(Ranges::RangeDescriptorT(10, 180.0f));

	Ranges::RangeTableT atA;
	atA.push_back(Ranges::RangeDescriptorT(1, 50.0f));
	atA.push_back(Ranges::RangeDescriptorT(2, 20.0f));
	atA.push_back(Ranges::RangeDescriptorT(3, 5.0f));
	atA.push_back(Ranges::RangeDescriptorT(5, 1.0f));
	atA.push_back(Ranges::RangeDescriptorT(8, 0.5f));
	atA.push_back(Ranges::RangeDescriptorT(10, 0.0f));

	Ranges ranges(dtC, soO, vel, direc, atA);

	BOOST_CHECK_EQUAL(ranges.dtC(0.3f), 1);
	BOOST_CHECK_EQUAL(ranges.dtC(35.3f), 2);
	BOOST_CHECK_EQUAL(ranges.dtC(150.3f), 4);
	BOOST_CHECK_EQUAL(ranges.dtC(350.3f), 10);
	
	BOOST_CHECK_EQUAL(ranges.soO(201.3f), 1);
	BOOST_CHECK_EQUAL(ranges.soO(150.0f), 1);
	BOOST_CHECK_EQUAL(ranges.soO(80.5f), 2);
	BOOST_CHECK_EQUAL(ranges.soO(49.9f), 4);
	BOOST_CHECK_EQUAL(ranges.soO(25.1f), 6);
	BOOST_CHECK_EQUAL(ranges.soO(3.2f), 8);
	
	// not logic possible but this doesn't matters here.
	BOOST_CHECK_EQUAL(ranges.soO(-1.0f), 10);

	BOOST_CHECK_EQUAL(ranges.velocity(301.0f), 1);
	BOOST_CHECK_EQUAL(ranges.velocity(299.9f), 1);
	BOOST_CHECK_EQUAL(ranges.velocity(200.1f), 1);
	BOOST_CHECK_EQUAL(ranges.velocity(200.0f), 2);
	BOOST_CHECK_EQUAL(ranges.velocity(180.1f), 2);

	BOOST_CHECK_EQUAL(ranges.direction(0.0f), 1);
	BOOST_CHECK_EQUAL(ranges.direction(3.0f), 1);
	BOOST_CHECK_EQUAL(ranges.direction(183.0f), 10);
	

	BOOST_CHECK_EQUAL(ranges.atA(30.0f), 1);
}

BOOST_AUTO_TEST_CASE (LoDLoadFromXmlTest)
{
	using namespace BFG;

	BOOST_CHECK_NO_THROW(Ranges ranges);
	BOOST_CHECK_NO_THROW(Quantifier quantifier);

	Ranges ranges;
	Quantifier quantifier;

	BOOST_CHECK_NO_THROW(ranges.reload());
	BOOST_CHECK_NO_THROW(quantifier.reload());
}


BOOST_AUTO_TEST_SUITE_END()
