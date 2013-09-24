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
#include <Event/Catcher.h>

#include <Model/Property/Concepts/AutoNavigator.h>

#include <boost/test/unit_test.hpp>

#include "Utils.h"

BOOST_AUTO_TEST_SUITE(AutoNavigatorTestSuite)

BOOST_AUTO_TEST_CASE (CreateObjectTest)
{
	using namespace BFG;
	
	// Event System setup
	Event::Synchronizer sync;
	Event::Lane lane(sync, 100);

	boost::shared_ptr<GameObject> go;
	boost::shared_ptr<SpacePlugin> spacePlugin;
	Property::PluginMapT pluginMap;
	BFG::PluginId spId = BFG::Property::ValueId::ENGINE_PLUGIN_ID;
	Module::ValueStorageT vs;
	BFG::GameHandle handle = BFG::generateHandle();
	BFG::AutoNavigator::TargetContainerT targets;
	targets.push_back(handle);
	vs[Property::ValueId(ID::PV_FirstTargets, spId)] = targets;
	boost::tie(go, spacePlugin) = createTestGameObject(lane, pluginMap, handle, vs);
	AutoNavigator autoNavigator(*go, spacePlugin->id());
	
	sync.start();
	sync.finish();

	Event::Catcher<f32> cPitch(lane, ID::GOE_CONTROL_PITCH, handle);
	Event::Catcher<f32> cYaw(lane, ID::GOE_CONTROL_YAW, handle);
	Event::Catcher<f32> cThrust(lane, ID::GOE_CONTROL_THRUST, handle);

	autoNavigator.synchronize();
	
	sync.start();
	sync.finish();

	BOOST_CHECK_EQUAL(cPitch.count(), 1);
	BOOST_CHECK_EQUAL(cYaw.count(), 1);
	BOOST_CHECK_EQUAL(cThrust.count(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
