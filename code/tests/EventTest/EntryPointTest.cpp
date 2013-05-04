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

#include <Base/LibraryMainBase.h>
#include <EventSystem/Core/EventLoop.h>

#include <boost/test/unit_test.hpp>

bool EntryPointCalled = false;
bool CodeConstructed = false;
bool CodeDestroyed = false;
bool MainDestroyed = false;

const int CodeParamOriginal = 123;
int       CodeParamResult = 0;

// Test Code
// ##########

namespace Lib {

struct Code
{
	Code(int param)
	{
		CodeConstructed = true;
		CodeParamResult = param;
	}
	~Code()
	{
		CodeDestroyed = true;
	}
};

struct Main : BFG::Base::LibraryMainBase<EventLoop>
{
	Main(int param) :
	mParam(param)
	{}

	~Main()
	{
		MainDestroyed = true;
	}

	virtual void main(EventLoop*)
	{
		EntryPointCalled = true;
		mCode.reset(new Code(mParam));
	}

private:
	int mParam;
	boost::scoped_ptr<Code> mCode;
};

} // namespace Lib

// Unit Tests
// ###########

BOOST_AUTO_TEST_SUITE(EntryPointTestSuite)

BOOST_AUTO_TEST_CASE (testMain)
{
	EventLoop loop(false);

	Lib::Main main(CodeParamOriginal);
	loop.addEntryPoint(main.entryPoint());
	
	// Testing
	loop.callEntryPoints(&loop);

	BOOST_CHECK(EntryPointCalled);
	BOOST_CHECK(CodeConstructed);
	BOOST_CHECK_EQUAL(CodeParamResult, CodeParamOriginal);
}

BOOST_AUTO_TEST_CASE (testCleanup)
{
	BOOST_CHECK(CodeDestroyed);
	BOOST_CHECK(MainDestroyed);
}

BOOST_AUTO_TEST_SUITE_END()
