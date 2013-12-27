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

#include <Model/Property/Plugin.h>
#include "Utils.h"

#define BOOST_TEST_MODULE ModelTest
#include <boost/test/unit_test.hpp>

const BFG::GameHandle handle1 = BFG::generateHandle();
const BFG::GameHandle handle2 = BFG::generateHandle();

BOOST_AUTO_TEST_SUITE(GameObjectTestSuite)

BOOST_AUTO_TEST_CASE (GameObjectTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	boost::shared_ptr<BFG::GameObject> go;
	boost::shared_ptr<BFG::SpacePlugin> spacePlugin;
	BFG::Property::PluginMapT pluginMap;
	boost::tie(go, spacePlugin) = createTestGameObject(lane, pluginMap);
	BFG::PluginId spId = spacePlugin->id();
	
	sync.start();
	sync.finish();

	// test getValue()
 	BOOST_CHECK_EQUAL(go->getValue<bool>(BFG::ID::PV_Remote, spId), false);
 
	// test getValue() with invalid values
 	BOOST_CHECK_THROW(go->getValue<bool>(BFG::ID::PV_Armor, spId), std::runtime_error);
 	BOOST_CHECK_THROW(go->getValue<BFG::u32>(BFG::ID::PV_Remote, spId), boost::bad_any_cast);
	
	// test docked()
	BOOST_CHECK_EQUAL(go->docked(), false);
	
	// test environment()
	BOOST_CHECK(go->environment() != nullptr);
	
	// test rootAdapters()
	BOOST_CHECK(go->rootAdapters().empty());
	
	// test plugins()
	BOOST_CHECK_EQUAL(go->plugins().size(), 1);

	// test getHandle()
	BOOST_CHECK_NE(go->getHandle(), BFG::NULL_HANDLE);
	
	// test getPublicName()
	BOOST_CHECK(!go->getPublicName().empty());
	
	// test getObjectType()
	BOOST_CHECK_EQUAL(go->getObjectType(), BFG::ID::OT_GameObject);
	
	// test attachModule
	// test attachModule
	// test detachModule
	// test satisfiesRequirement
	// test debugOutput
}

BOOST_AUTO_TEST_CASE (VectorToModuleFromRootTest)
{
	// Absolut random values
	BFG::Adapter a;
	a.mParentPosition = v3(0.1f, 0.2f, 0.3f);
	a.mChildPosition = v3(0.4f, 0.5f, 0.6f);
	a.mParentOrientation = BFG::qv4(0.707f, 0.0f, 0.707f, 0.0f);
	a.mChildOrientation = BFG::qv4(0.707f, 0.0f, 0.0f, 0.707f);

	BFG::Adapter b;
	b.mParentPosition = v3(0.7f, 0.8f, 0.9f);
	b.mChildPosition = v3(1.0f, 1.1f, 1.2f);
	b.mParentOrientation = BFG::qv4(0.910f, 0.109f, 0.260f, 0.305f);
	b.mChildOrientation = BFG::qv4(0.942f, 0.260f, 0.209f, 0.0363f);

	BFG::Adapter c;
	c.mParentPosition = v3(1.3f, 1.4f, 1.5f);
	c.mChildPosition = v3(1.6f, 1.7f, 1.8f);
	c.mParentOrientation = BFG::qv4(0.952f, 0.239f, 0.189f, 0.0381f);
	c.mChildOrientation = BFG::qv4(0.764f, 0.570f, 0.264f, -0.150f);

	std::vector<BFG::Adapter> adapters;
	adapters.push_back(a);
	adapters.push_back(b);
	adapters.push_back(c);

	BFG::v3 position;
	BFG::qv4 orientation;

	BFG::vectorToModuleFromRoot(adapters, position, orientation);

	// Precalculated with the working algorithm
	BFG::v3 expectedPos(0.9556f, 3.87371f, 0.599187f);
	BFG::qv4 expectedOri(0.221768f, 0.63988f, -0.017174f, 0.735102f);

	const BFG::f32 ERROR_MARGIN = 0.0001f;

	bool xPosCorrect = std::fabs(position.x - expectedPos.x) < ERROR_MARGIN;
	bool yPosCorrect = std::fabs(position.y - expectedPos.y) < ERROR_MARGIN;
	bool zPosCorrect = std::fabs(position.z - expectedPos.z) < ERROR_MARGIN;
	bool positionCorrect = xPosCorrect && yPosCorrect && zPosCorrect;

	bool wOriCorrect = std::fabs(orientation.w - expectedOri.w) < ERROR_MARGIN;
	bool xOriCorrect = std::fabs(orientation.x - expectedOri.x) < ERROR_MARGIN;
	bool yOriCorrect = std::fabs(orientation.y - expectedOri.y) < ERROR_MARGIN;
	bool zOriCorrect = std::fabs(orientation.z - expectedOri.z) < ERROR_MARGIN;
	bool orientationCorrect = wOriCorrect && xOriCorrect && yOriCorrect && zOriCorrect;

	BOOST_REQUIRE (positionCorrect && orientationCorrect);
}

BOOST_AUTO_TEST_SUITE_END()
