/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

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

/**
	@file

	This example application demonstrates how to create a very simple
	render window which reacts on input from Keyboard and Mouse.
*/

#include <boost/foreach.hpp>

// OGRE
#include <OgreException.h>

// BFG libraries
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Base/ShowException.h>

#include <Controller/Controller.h>

#include <Core/Path.h>
#include <Core/GameHandle.h>

#include <Event/Event.h>

#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Data/GameObjectFactory.h>
#include <Model/Sector.h>
#include <Model/Property/Concept.h>
#include <Model/Property/Plugin.h>
#include <Model/Property/SpacePlugin.h>
#include <Model/State.h>

#include <Network/Network.h>

#include <Physics/Physics_fwd.h>

#include <View/View.h>

using namespace boost::units;

using BFG::s32;
using BFG::f32;

const s32 A_EXIT = 10000;
const s32 SIMULATION_0 = 10001;
const s32 SIMULATION_1 = 10002;
const s32 SIMULATION_2 = 10003;
const s32 SIMULATION_3 = 10004;
const s32 A_CONSOLE = 10005;
const s32 CREATE_TEST_OBJECT = 15000;
const s32 START_SIMULATION_0 = 15001;
const s32 START_SIMULATION_1 = 15002;
const s32 START_SIMULATION_2 = 15003;
const s32 START_SIMULATION_3 = 15004;

const GameHandle SERVER_STATE_HANDLE = 42;
const GameHandle CLIENT_STATE_HANDLE = 43;

const s32 ROWS = 3;

bool alwaysTrue(boost::shared_ptr<BFG::GameObject>)
{
	return true;
}

void initController(BFG::GameHandle stateHandle, BFG::Event::Lane& lane)
{
	BFG::Controller_::ActionMapT actions;
	actions[A_EXIT] = "A_EXIT";
	actions[SIMULATION_0] = "SIMULATION_0";
	actions[SIMULATION_1] = "SIMULATION_1";
	actions[SIMULATION_2] = "SIMULATION_2";
	actions[SIMULATION_3] = "SIMULATION_3";
	actions[A_CONSOLE] = "A_CONSOLE";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(lane, actions);

	BFG::Path path;
	const std::string configPath = path.Expand("SynchronizationTest.xml");
	const std::string stateName = "SynchronizationTest";

	BFG::View::WindowAttributes wa;
	BFG::View::queryWindowAttributes(wa);
	
	BFG::Controller_::StateInsertion si(configPath, stateName, stateHandle, true, wa);
	lane.emit(BFG::ID::CE_LOAD_STATE, si);
}

struct SynchronizationTestState: BFG::State
{
	SynchronizationTestState(GameHandle handle, BFG::Event::Lane& lane) :
	State(lane),
	mSubLane(lane.createSubLane()),
	mStateHandle(handle),
	mPlayer(BFG::NULL_HANDLE),
	mEnvironment(new BFG::Environment)
	{
		BFG::Path p;
		std::string def = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "default/";

		BFG::LevelConfig lc;

		lc.mModules.push_back(def + "Object.xml");
		lc.mAdapters.push_back(def + "Adapter.xml");
		lc.mConcepts.push_back(def + "Concept.xml");
		lc.mProperties.push_back(def + "Value.xml");

		using BFG::Property::ValueId;

		BFG::PluginId spId = ValueId::ENGINE_PLUGIN_ID;
		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
		mPluginMap.insert(sp);

		mGof.reset(new BFG::GameObjectFactory(lane, lc, mPluginMap, mEnvironment, mStateHandle));

		mSector.reset(new BFG::Sector(lane, 1, "Blah", mGof));
	}

	virtual ~SynchronizationTestState()
	{}

	virtual void onTick(const quantity<si::time, f32> TSLF)
	{
		mSector->update(TSLF);
	}

	virtual void createObject(const BFG::ObjectParameter& param)
	{
		boost::shared_ptr<BFG::GameObject> playerShip = mGof->createGameObject(param);
		mSector->addObject(playerShip);
		mSubLane->emit(BFG::ID::GOE_GHOST_MODE, true, param.mHandle);
		mPlayer = playerShip->getHandle();
	}

	virtual void destroyObject(GameHandle handle)
	{
		mSector->removeObject(handle);
	}

protected:
	BFG::Event::SubLanePtr mSubLane;
	GameHandle mStateHandle;
	GameHandle mPlayer;
	boost::shared_ptr<BFG::Environment> mEnvironment;

private:
	BFG::Property::PluginMapT mPluginMap;
	boost::shared_ptr<BFG::Sector> mSector;
	boost::shared_ptr<BFG::GameObjectFactory> mGof;

};

struct ServerState: public SynchronizationTestState
{
	typedef std::vector<BFG::Network::PeerIdT> ClientListT;

	ServerState(GameHandle handle, BFG::Event::Lane& lane) :
	SynchronizationTestState(handle, lane),
	mSceneCreated(false)
	{
		mSubLane->connect(BFG::ID::NE_RECEIVED, this, &ServerState::onReceived, SERVER_STATE_HANDLE);
		mSubLane->connect(BFG::ID::NE_CONNECTED, this, &ServerState::onConnected);
		mSubLane->connect(BFG::ID::NE_DISCONNECTED, this, &ServerState::onDisconnected);
	}
	
	virtual ~ServerState()
	{}

	void onReceived(const BFG::Network::DataPayload& payload)
	{
		switch(payload.mAppEventId)
		{
		case START_SIMULATION_0:
		{
			infolog << "Starting Simulation 0";
			
			std::vector<GameHandle> all = mEnvironment->find_all(&alwaysTrue);
			std::vector<GameHandle>::const_iterator it = all.begin();
			for (; it != all.end(); ++it)
			{
				mSubLane->emit(BFG::ID::PE_DEBUG, BFG::Event::Void(), *it);
				mSubLane->emit(BFG::ID::PE_UPDATE_VELOCITY, v3::ZERO, *it);
				mSubLane->emit(BFG::ID::PE_UPDATE_ROTATION_VELOCITY, v3::ZERO, *it);
			}
			break;
		}
		case START_SIMULATION_1:
		{
			infolog << "Starting Simulation 1";
			v3 position = v3(2.0f, -1.0f, 50.0f);
			mSubLane->emit(BFG::ID::PE_UPDATE_POSITION, position, mPlayer);

			break;
		}
		case START_SIMULATION_2:
		{
			infolog << "Starting Simulation 2";
			v3 force = v3(-1000000.0f, 0.0f, 0.0f);
			mSubLane->emit(BFG::ID::PE_APPLY_FORCE, force, mPlayer);

			break;
		}
		case START_SIMULATION_3:
		{
			infolog << "Starting Simulation 3";
			v3 torque = v3(5000.0f, 0.0f, 0.0f); // spin around the x-axis
			mSubLane->emit(BFG::ID::PE_APPLY_TORQUE, torque, mPlayer);

			break;
		}
		}
	}
	
	void createScene()
	{
		std::stringstream handles;

		// First cube
		BFG::ObjectParameter op;
		op.mType = "Cube";

		f32 width = 1.0f;
		f32 height = 1.0f;
		f32 offset = 0.5f;

		int counter = 0;

		for (int i = 0; i < ROWS; ++i)
		{
			for(int j = 0; j <= i; ++j)
			{
				f32 x = - i * (width + offset) - 5.0f;
				f32 y = j * (height + offset) - (i * (height + offset) / 2.0f);

				std::stringstream number;
				number << ++counter;
				op.mHandle = BFG::generateNetworkHandle();
				op.mName = "TestCube" + number.str();
				op.mLocation.position = v3(x, y, 50.0f);
				if (i == j && i == (ROWS-1))
					handles << op.mHandle;
				else
					handles << op.mHandle << " ";

				createObject(op);
				mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);
			}
		}
		mCreatedHandles = handles.str();
		mSceneCreated = true;
	}

	void destroyScene()
	{
		std::vector<GameHandle> all = mEnvironment->find_all(&alwaysTrue);
		std::vector<GameHandle>::const_iterator it = all.begin();
		for (; it != all.end(); ++it)
		{
			destroyObject(*it);
		}
		
		mCreatedHandles.clear();
		mSceneCreated = false;
	}

	void onConnected(BFG::Network::PeerIdT peerId)
	{
		dbglog << "Client (" << peerId << ") wants to connect";
		ClientListT::iterator it = std::find(mClientList.begin(), mClientList.end(), peerId);

		if (it != mClientList.end())
		{
			errlog << "Client with the same PeerID (" << peerId << ") already connected!";
			return;
		}
			
		mClientList.push_back(peerId);

		if (!mSceneCreated)
			createScene();
			
		CharArray512T ca512 = stringToArray<512>(mCreatedHandles);

		BFG::Network::DataPayload payload
		(
			CREATE_TEST_OBJECT, 
			CLIENT_STATE_HANDLE, 
			SERVER_STATE_HANDLE,
			mCreatedHandles.length(),
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload, peerId);
	}

	void onDisconnected(BFG::Network::PeerIdT peerId)
	{
		ClientListT::iterator it = std::find(mClientList.begin(), mClientList.end(), peerId);

		if (it == mClientList.end())
		{
			errlog << "Client (" << peerId << ") was not connected!";
			return;
		}

		mClientList.erase(it);

		if (mClientList.empty())
		{
			destroyScene();
		}
	}

private:
	ClientListT mClientList;

	std::string mCreatedHandles;
	bool mSceneCreated;
};

struct ClientState : public SynchronizationTestState
{
	ClientState(GameHandle handle, BFG::Event::Lane& lane) :
	SynchronizationTestState(handle, lane)
	{
		mSubLane->connect(A_EXIT, this, &ClientState::onExit);
		mSubLane->connect(SIMULATION_0, this, &ClientState::onSimulation0);
		mSubLane->connect(SIMULATION_1, this, &ClientState::onSimulation1);
		mSubLane->connect(SIMULATION_2, this, &ClientState::onSimulation2);
		mSubLane->connect(SIMULATION_3, this, &ClientState::onSimulation3);
		mSubLane->connect(A_CONSOLE, this, &ClientState::onConsole);
		mSubLane->connect(BFG::ID::NE_RECEIVED, this, &ClientState::onReceived, CLIENT_STATE_HANDLE);
		
		initController(CLIENT_STATE_HANDLE, lane);
	}

	virtual ~ClientState()
	{
		infolog << "Tutorial: Destroying GameState.";
	}

	void onExit(s32)
	{
		mSubLane->emit(BFG::ID::EA_FINISH, BFG::Event::Void());
	}
	
	void onSimulation0(s32)
	{
		std::vector<GameHandle> all = mEnvironment->find_all(alwaysTrue);
		for (std::size_t i=0; i<all.size(); ++i)
			mSubLane->emit(BFG::ID::PE_DEBUG, BFG::Event::Void(), all[i]);
		
		CharArray512T ca512 = CharArray512T();
		BFG::Network::DataPayload payload
		(
			START_SIMULATION_0, 
			SERVER_STATE_HANDLE, 
			CLIENT_STATE_HANDLE,
			0,
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload);
	}

	void onSimulation1(s32)
	{
		CharArray512T ca512 = CharArray512T();
		BFG::Network::DataPayload payload
		(
			START_SIMULATION_1, 
			SERVER_STATE_HANDLE, 
			CLIENT_STATE_HANDLE,
			0,
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload);
	}
	
	void onSimulation2(s32)
	{
		CharArray512T ca512 = CharArray512T();
		BFG::Network::DataPayload payload
		(
			START_SIMULATION_2, 
			SERVER_STATE_HANDLE, 
			CLIENT_STATE_HANDLE,
			0,
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload);
	}

	void onSimulation3(s32)
	{
		CharArray512T ca512 = CharArray512T();
		BFG::Network::DataPayload payload
		(
			START_SIMULATION_3, 
			SERVER_STATE_HANDLE, 
			CLIENT_STATE_HANDLE,
			0,
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload);
	}
	
	void onConsole(bool show)
	{
		mSubLane->emit(BFG::ID::VE_CONSOLE, show);
	}
	
	void onCreateTestObject(const BFG::Network::DataPayload& payload)
	{
		std::stringstream oss(payload.mAppData.data());

		// First cube
		BFG::ObjectParameter op;
		op.mType = "Cube";

		f32 width = 1.0f;
		f32 height = 1.0f;
		f32 offset = 0.5f;

		int counter = 0;

		for (int i = 0; i < ROWS; ++i)
		{
			for(int j = 0; j <= i; ++j)
			{
				f32 x = - i * (width + offset) - 5.0f;
				f32 y = j * (height + offset) - (i * (height + offset) / 2.0f);

				std::stringstream number;
				number << ++counter;
				oss >> op.mHandle;
				op.mName = "TestCube" + number.str();
				op.mLocation.position = v3(x, y, 50.0f);

				createObject(op);
				mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);
			}
		}
	}
	
	void onReceived(const BFG::Network::DataPayload& payload)
	{
		switch(payload.mAppEventId)
		{
		case CREATE_TEST_OBJECT:
		{
			onCreateTestObject(payload);
			break;
		}
		}
	}
};

struct ViewState : public BFG::View::State
{
public:
	ViewState(GameHandle handle, BFG::Event::Lane& lane) :
	State(handle, lane),
	mLane(lane),
	mControllerMyGuiAdapter(handle, lane)
	{
		dbglog << "SynchronizationTest: Creating ViewState.";
		lane.emit(BFG::ID::VE_SET_AMBIENT, BFG::cv4(1.0f, 1.0f, 1.0f), handle);
	}

	virtual ~ViewState()
	{
		dbglog << "SynchronizationTest: Destroying ViewState.";
		mLane.emit(BFG::ID::VE_SHUTDOWN, BFG::Event::Void());
	}

	virtual void pause()
	{}

	virtual void resume()
	{}
	
private:
	BFG::Event::Lane& mLane;
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

struct ClientMain : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	ClientMain()
	{}
	
	virtual void main(BFG::Event::Lane* lane)
	{
		mClientState.reset(new ClientState(CLIENT_STATE_HANDLE, *lane));
	}
	
	boost::scoped_ptr<ClientState> mClientState;
};

struct ClientViewMain : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	ClientViewMain()
	{}

	virtual void main(BFG::Event::Lane* lane)
	{
		mViewState.reset(new ViewState(CLIENT_STATE_HANDLE, *lane));
	}

	boost::scoped_ptr<ViewState> mViewState;
};

struct ServerMain : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	ServerMain()
	{}
	
	virtual void main(BFG::Event::Lane* lane)
	{
		mServerState.reset(new ServerState(SERVER_STATE_HANDLE, *lane));
	}
	
	boost::scoped_ptr<ServerState> mServerState;
};

template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

int main( int argc, const char* argv[] ) try
{
	bool server = false;
	if  (argc == 2)
		server = true;
	else if (argc == 3)
		server = false;
	else
	{
		std::cerr << "For Server use: bfgSynchronizationTest <Port>\nFor Client use: bfgSynchronizationTest <IP> <Port>\n";
		BFG::Base::pause();
		return 0;
	}	

	BFG::Base::Logger::SeverityLevel level = BFG::Base::Logger::SL_DEBUG;
	//BFG::Base::Logger::SeverityLevel level = BFG::Base::Logger::SL_ERROR;
	
	if (server)
	{
		BFG::Path p;
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/SynchronizationTestServer.log");

		BFG::u16 port = 0;

		if (!from_string(port, argv[1], std::dec))
		{
			std::cerr << "Port not a number: " << argv[1] << std::endl;
			BFG::Base::pause();
			return 0;
		}

		BFG::Event::Synchronizer synchronizer;
		BFG::Event::Lane networkLane(synchronizer, 100, "Network");
		BFG::Event::Lane viewLane(synchronizer, 100, "View");
		BFG::Event::Lane physicsLane(synchronizer, 100, "Physics", BFG::Event::RL2);
		BFG::Event::Lane serverLane(synchronizer, 100, "Server", BFG::Event::RL3);
		
		networkLane.addEntry<BFG::Network::Main>(BFG_SERVER);
		physicsLane.addEntry<BFG::Physics::Main>();
		viewLane.addEntry<BFG::View::MiniMain>();
		serverLane.addEntry<ServerMain>();
		
		synchronizer.start();
		
		networkLane.emit(BFG::ID::NE_LISTEN, port);
		
		BFG::Base::pause();
		synchronizer.finish();
	}
	else
	{
		std::string ip(argv[1]);
		std::string port(argv[2]);

		BFG::Path p;
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/SynchronizationTestClient.log");

		size_t controllerFrequency = 1000;

		BFG::Event::Synchronizer synchronizer;
		BFG::Event::Lane networkLane(synchronizer, 100, "Network");
		BFG::Event::Lane physicsLane(synchronizer, 100, "Physics", BFG::Event::RL2);
		BFG::Event::Lane viewLane(synchronizer, 100, "View");
		BFG::Event::Lane controllerLane(synchronizer, controllerFrequency, "Controller");
		BFG::Event::Lane clientLane(synchronizer, 100, "Client", BFG::Event::RL3);
		
		networkLane.addEntry<BFG::Network::Main>(BFG_CLIENT);
		physicsLane.addEntry<BFG::Physics::Main>();
		viewLane.addEntry<BFG::View::Main>("SynchronizationTest");
		viewLane.addEntry<ClientViewMain>();
		controllerLane.addEntry<BFG::Controller_::Main>(controllerFrequency);
		clientLane.addEntry<ClientMain>();
		
		synchronizer.start();
		
		BFG::Network::EndpointT payload = boost::make_tuple(stringToArray<128>(ip), stringToArray<128>(port));
		networkLane.emit(BFG::ID::NE_CONNECT, payload);
		
		synchronizer.finish(true);
	}
	
	infolog << "Good Bye!";
}
catch (Ogre::Exception& e)
{
	BFG::showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	BFG::showException(ex.what());
}
catch (...)
{
	BFG::showException("Unknown exception");
}
