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

#include <boost/test/unit_test.hpp>

#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Data/GameObjectFactory.h>
#include <Model/Property/SpacePlugin.h>

const BFG::GameHandle handle1 = BFG::generateHandle();
const BFG::GameHandle handle2 = BFG::generateHandle();

bool findGo(boost::shared_ptr<BFG::GameObject>)
{
	return true;
}

bool findGo1(boost::shared_ptr<BFG::GameObject> go)
{
	return go->getHandle() == handle1;
}

bool findGo2(boost::shared_ptr<BFG::GameObject> go)
{
	return go->getHandle() == handle2;
}

BOOST_AUTO_TEST_SUITE(EnvironmentTestSuite)

BOOST_AUTO_TEST_CASE (EnvironmentTest)
{
	BFG::Property::PluginMapT pluginMap;
	BFG::PluginId spId = BFG::Property::ValueId::ENGINE_PLUGIN_ID;
	boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
	pluginMap.insert(sp);

	boost::shared_ptr<BFG::Environment> environment(new BFG::Environment());

	BFG::Event::Synchronizer sync;
	BFG::Event::Lane lane(sync, 100);
	
	BFG::Module::ValueStorageT emptyValues;

	emptyValues.insert(std::make_pair(BFG::ValueId(BFG::ID::PV_Position, spId), BFG::v3::ZERO));
	emptyValues.insert(std::make_pair(BFG::ValueId(BFG::ID::PV_Orientation, spId), BFG::qv4::IDENTITY));
	emptyValues.insert(std::make_pair(BFG::ValueId(BFG::ID::PV_Velocity, spId), BFG::v3::ZERO));

	boost::shared_ptr<BFG::GameObject> go(new BFG::GameObject(lane, BFG::NULL_HANDLE, "TestObject", emptyValues, pluginMap, environment));

	// add null handle object
	BOOST_CHECK_EQUAL(environment->exists(BFG::NULL_HANDLE), false);
	BOOST_CHECK_THROW(environment->addGameObject(go), std::logic_error);
	BOOST_CHECK_EQUAL(environment->exists(BFG::NULL_HANDLE), false);

	BOOST_CHECK_NO_THROW(environment->removeGameObject(BFG::NULL_HANDLE));

	go.reset(new BFG::GameObject(lane, handle1, "TestObject", emptyValues, pluginMap, environment));

	// add normal object
	BOOST_CHECK_NO_THROW(environment->addGameObject(go));
	BOOST_CHECK_EQUAL(environment->exists(handle1), true);

	// destroy old object and create new
	go.reset(new BFG::GameObject(lane, handle2, "TestObject2", emptyValues, pluginMap, environment));
	BOOST_CHECK_NO_THROW(environment->addGameObject(go));
	BOOST_CHECK_EQUAL(environment->exists(handle2), true);

	// old object was only weak pointer
	BOOST_CHECK_EQUAL(environment->exists(handle1), false);
	BOOST_CHECK_THROW(environment->getGoValue<bool>(handle1, BFG::ID::PV_Remote, spId), std::logic_error);

	// create first object again
	boost::shared_ptr<BFG::GameObject> go2(new BFG::GameObject(lane, handle1, "TestObject", emptyValues, pluginMap, environment));
	BOOST_CHECK_NO_THROW(environment->addGameObject(go2));
	BOOST_CHECK_EQUAL(environment->exists(handle1), true);
	BOOST_CHECK_EQUAL(environment->exists(handle2), true);

	// test nextHandle
	BOOST_CHECK_EQUAL(environment->nextHandle(BFG::NULL_HANDLE), handle1);
	BOOST_CHECK_EQUAL(environment->nextHandle(handle1), handle2);
	BOOST_CHECK_EQUAL(environment->nextHandle(handle2), handle1);

	// test prevHandle
	BOOST_CHECK_EQUAL(environment->prevHandle(BFG::NULL_HANDLE), handle2);
	BOOST_CHECK_EQUAL(environment->prevHandle(handle2), handle1);
	BOOST_CHECK_EQUAL(environment->prevHandle(handle1), handle2);

	// test find not successful (GameObjects aren't activated yet)
	BOOST_CHECK_EQUAL(environment->find(&findGo1), BFG::NULL_HANDLE);
	BOOST_CHECK_EQUAL(environment->find(&findGo2), BFG::NULL_HANDLE);

	go->activate();
	go2->activate();
	
	// test find successful
	BOOST_CHECK_EQUAL(environment->find(&findGo1), handle1);
	BOOST_CHECK_EQUAL(environment->find(&findGo2), handle2);

	// test find all
	std::vector<BFG::GameHandle> handleVec;
	handleVec.push_back(handle1);
	handleVec.push_back(handle2);

	std::vector<BFG::GameHandle> resultVec = environment->find_all(&findGo);
	BOOST_CHECK_EQUAL_COLLECTIONS(resultVec.begin(), resultVec.end(), handleVec.begin(), handleVec.end());

	// test remove object
	BOOST_CHECK_NO_THROW(environment->removeGameObject(handle1));
	BOOST_CHECK_EQUAL(environment->find(&findGo1), BFG::NULL_HANDLE);

	BOOST_CHECK_NO_THROW(environment->removeGameObject(handle2));
	BOOST_CHECK_EQUAL(environment->find(&findGo2), BFG::NULL_HANDLE);
}

BOOST_AUTO_TEST_SUITE_END()