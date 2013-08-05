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

#include <Event/Event.h>

#include <boost/test/unit_test.hpp>

struct NonCopyData : boost::noncopyable
{};

struct Data
{};

struct MainParameterByValue : BFG::Event::EntryPoint<BFG::Event::Lane>
{
	MainParameterByValue(Data)
	{}
};

struct MainParameterByIntValue : BFG::Event::EntryPoint<BFG::Event::Lane>
{
	MainParameterByIntValue(BFG::u32)
	{}
};

struct MainParameterByReference : BFG::Event::EntryPoint<BFG::Event::Lane>
{
	MainParameterByReference(Data&)
	{}
	
	MainParameterByReference(NonCopyData&)
	{}
};

struct MainParameterByConstReference : BFG::Event::EntryPoint<BFG::Event::Lane>
{
	MainParameterByConstReference(const NonCopyData&)
	{}
};

// ---------------------------------------------------------------------------

struct MainThrowing : BFG::Event::EntryPoint<BFG::Event::Lane>
{
	MainThrowing()
	{
	}
	
	virtual void run(BFG::Event::Lane* lane)
	{
		throw std::runtime_error("MainThrowing::main");
	}
};

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TestSuite)

//! Testing the correct emit order
BOOST_AUTO_TEST_CASE (AddEntry)
{
	// Logger init may be done within the first test.
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "EventTest.log");
	
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	Data data;
	NonCopyData ndata;
	BFG::u32 fifteen = 15;
	
	// by value
	lane.addEntry<MainParameterByValue>(Data());    // copy
	lane.addEntry<MainParameterByValue>(data);      // copy
	
	lane.addEntry<MainParameterByIntValue>(fifteen); // copy
	lane.addEntry<MainParameterByIntValue>(15);      // rvalue
	
	lane.addEntry<MainParameterByReference>(boost::ref(ndata)); // reference
	lane.addEntry<MainParameterByReference>(Data());            // rvalue
	//lane.addEntry<MainParameterByReference>(NonCopyData()); // Disabled: Needs rvalue references
	
	lane.addEntry<MainParameterByConstReference>(boost::cref(NonCopyData()));
	lane.addEntry<MainParameterByConstReference>(boost::cref(ndata));
	
	sync.finish();
}

BOOST_AUTO_TEST_CASE (AddEntryException)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	lane.addEntry<MainThrowing>();
	sync.startEntries();
	sync.finish();
}

BOOST_AUTO_TEST_SUITE_END()
