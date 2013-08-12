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

#define BOOST_TEST_MODULE ViewTest
#include <boost/test/unit_test.hpp>

#include <OgreRoot.h>

#include <Base/Logger.h>
#include <Core/Path.h>
#include <Event/Event.h>
#include <View/Main.h>
#include <View/Enums.hh>

// ---------------------------------------------------------------------------
void shutdown(BFG::u32 countdown)
{
	std::cout << "Shutdown in " << countdown;
	for (BFG::u32 i = countdown; i > 0; --i)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		std::cout << ", " << i-1;
	}

	std::cout << std::endl;
}

BOOST_AUTO_TEST_SUITE(ViewTestSuite)
#if 0
BOOST_AUTO_TEST_CASE (OgreTest)
{
#ifdef _WIN32
	const std::string pluginCfg = "plugins-win.cfg";
#else
	const std::string pluginCfg = "plugins-linux.cfg";
#endif

	BFG::Path p;

	Ogre::Root* ogreRoot = new Ogre::Root(pluginCfg, "ogre.cfg", p.Get(BFG::ID::P_LOGS) + "ogre.log");

	if(ogreRoot->restoreConfig() || ogreRoot->showConfigDialog())
	{
		ogreRoot->initialise(true, "OgreTest Window");
	}
	else
		throw std::runtime_error("OgreInit: failed to restore config dialog!");

	shutdown(5);
	
	ogreRoot->shutdown();

	delete ogreRoot;
}
#endif
BOOST_AUTO_TEST_CASE (testLibraryInit)
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/testLibraryInit.log");

	BFG::Event::Synchronizer sync;
	BFG::Event::Lane viewLane(sync, 100, "View");
	
	viewLane.addEntry<BFG::View::Main>("ViewTest Window");

	sync.start();
	
	shutdown(10);

	sync.finish();
}

BOOST_AUTO_TEST_SUITE_END()
