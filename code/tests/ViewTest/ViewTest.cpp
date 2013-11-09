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

#include <Base/Cpp.h>
#include <Base/Logger.h>
#include <Core/Mesh.h>
#include <Core/Path.h>
#include <Core/qv4.h>
#include <Event/Event.h>
#include <View/Effect.h>
#include <View/EffectCreation.h>
#include <View/Enums.hh>
#include <view/Explosion.h>
#include <View/Main.h>
#include <View/MiniMain.h>
#include <View/ObjectCreation.h>
#include <View/State.h>
#include <View/SkyCreation.h>

const BFG::GameHandle gViewState = 43;

// ---------------------------------------------------------------------------
void shutdownCountdown(BFG::u32 countdown)
{
	std::cout << std::endl << "Shutdown in " << countdown;
	for (BFG::u32 i = countdown; i > 0; --i)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		std::cout << ", " << i-1;
	}

	std::cout << std::endl;
}

struct ModuleDeliveryTest : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		std::cout << "TestModulView1-Run" << std::endl;

		lane->connect(BFG::ID::VE_DELIVER_MESH, this, &ModuleDeliveryTest::onDelivery);
	}

	void onDelivery(const BFG::NamedMesh& namedMesh)
	{
		const std::string& meshName = namedMesh.get<0>();
		const BFG::Mesh& mesh = namedMesh.get<1>();

		std::cout << "Mesh received: " << meshName << std::endl;
		std::cout << "verteces/indices: " << mesh.mVertexCount << "/" << mesh.mIndexCount << std::endl;

		std::cout << "Vertices:" << std::endl;
		
		for (BFG::u32 i = 0; i < mesh.mVertexCount; ++i)
		{
			std::cout << mesh.mVertices[i] << std::endl;
		}

		std::cout << "Indices:" << std::endl;
		for (BFG::u32 i = 0; i < mesh.mIndexCount; ++i)
		{
			std::cout << " " << mesh.mIndices[i];
		}

		std::cout << std::endl;
	}
};

struct ExplosionState : public BFG::View::State
{
	ExplosionState(BFG::Event::Lane* lane) :
	BFG::View::State(gViewState, *lane)
	{}

	virtual void pause(){}
	virtual void resume(){}
};

struct ModuleExplosionTest : public BFG::Event::EntryPoint<BFG::Event::Lane>
{
	void run(BFG::Event::Lane* lane)
	{
		std::cout << "ModuleExplosionTest-Run" << std::endl;
		
		s.reset(new ExplosionState(lane));
		lane->connect(BFG::ID::VE_EFFECT, this, &ModuleExplosionTest::onEffect);
	}

	void onEffect(const BFG::View::EffectCreation& ec)
	{
		if (ec.get<0>() == "Explosion_1")
			e.reset(new BFG::View::Explosion(ec.get<1>(), ec.get<2>()));
		else if (ec.get<0>() == "Explosion_2")
			e.reset(new BFG::View::Explosion2(ec.get<1>(), ec.get<2>()));
	}
	
	boost::shared_ptr<BFG::View::Effect> e;
	boost::shared_ptr<BFG::View::State> s;
};


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

	shutdownCountdown(5);
	
	ogreRoot->shutdown();

	delete ogreRoot;
}
#endif

BOOST_AUTO_TEST_CASE (testLibraryInit)
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/testLibraryInit.log");

	BFG::Event::Synchronizer sync;
	BFG::Event::Lane viewLane(sync, 100, "View");
	
	viewLane.addEntry<BFG::View::MiniMain>();

	sync.start();
	
	shutdownCountdown(3);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (meshTest)
{
	BFG::Event::Synchronizer sync;
	BFG::Event::Lane viewLane(sync, 100, "View");

	BFG::Event::Lane moduleLane(sync, 100, "Module", BFG::Event::RL2);

	viewLane.addEntry<BFG::View::MiniMain>();
	moduleLane.addEntry<ModuleDeliveryTest>();

	sync.start();

	viewLane.emit(BFG::ID::VE_REQUEST_MESH, std::string("Cube.mesh"));
	viewLane.emit(BFG::ID::VE_REQUEST_MESH, std::string("Rocket.mesh"));

	shutdownCountdown(3);

	sync.finish();
}

BOOST_AUTO_TEST_CASE (explosionTest)
{
// 	BFG::Path p;
// 	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, p.Get(BFG::ID::P_LOGS) + "/bfgViewTest.log");

	BFG::Event::Synchronizer sync;
	BFG::Event::Lane viewLane(sync, 100, "View");

	viewLane.addEntry<BFG::View::Main>("explosionTest");
	viewLane.addEntry<ModuleExplosionTest>();
	sync.start();
	
	BFG::View::SkyCreation sc("sky02");
	viewLane.emit(BFG::ID::VE_SET_SKY, sc, gViewState);

	BFG::v3 position(0.0f, 0.0f, 40.0f);
	BFG::View::ObjectCreation oc(BFG::NULL_HANDLE, BFG::generateHandle(), "Cube.mesh", position, BFG::qv4::IDENTITY);

	viewLane.emit(BFG::ID::VE_CREATE_OBJECT, oc, gViewState);

	for (int i = 0; i < 4; ++i)
	{
		viewLane.emit(BFG::ID::VE_EFFECT, BFG::View::EffectCreation("Explosion_1", BFG::v3(0.0f, 0.0f, 40.0f), 1.0f));
		boost::this_thread::sleep(boost::posix_time::milliseconds(9000));
	}
	for (int i = 0; i < 4; ++i)
	{
		viewLane.emit(BFG::ID::VE_EFFECT, BFG::View::EffectCreation("Explosion_2", BFG::v3(0.0f, 0.0f, 40.0f), 1.0f));
		boost::this_thread::sleep(boost::posix_time::milliseconds(4000));
	}

	shutdownCountdown(2);

	sync.finish();
}


BOOST_AUTO_TEST_SUITE_END()
