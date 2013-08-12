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

#include <Core/Path.h>
#include <Event/Event.h>
#include <View/Main.h>

// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ViewTestSuite)

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

	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));

	ogreRoot->shutdown();
}

#if 0
BOOST_AUTO_TEST_CASE (testLibraryInit)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane viewLane(sync, 100);
	
	viewLane.addEntry<BFG::View::Main>("ViewTest Window");

	sync.start();

	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));

	sync.finish();
}

#endif
BOOST_AUTO_TEST_SUITE_END()
