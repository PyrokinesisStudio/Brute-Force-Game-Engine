
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

#include <Core/GameHandle.h>
#include <Core/Math.h>
#include <Core/Path.h>

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

#include <Pong/PongDefinitions.h>
#include <Pong/PropertyPlugin.h>

using namespace boost::units;

using BFG::s32;
using BFG::f32;

const GameHandle SERVER_STATE_HANDLE = 42;
const GameHandle CLIENT_STATE_HANDLE = 43;


bool alwaysTrue(boost::shared_ptr<BFG::GameObject>)
{
	return true;
}

void initController(BFG::GameHandle stateHandle, BFG::Event::Lane& lane)
{
	BFG::Controller_::ActionMapT actions;
	actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
	actions[A_FPS]         = "A_FPS";
	actions[A_QUIT]        = "A_QUIT";
	actions[A_CONSOLE]     = "A_CONSOLE";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(lane, actions);

	BFG::Path path;
	const std::string configPath = path.Expand("Pong.xml");
	const std::string stateName = "Pong";

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
	mPlayer1(BFG::NULL_HANDLE),
	mPlayer2(BFG::NULL_HANDLE),
	mEnvironment(new BFG::Environment)
	{
		BFG::Path p;
		std::string def = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "default/";
		std::string level = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "pong/";

		BFG::LevelConfig lc;

		lc.mModules.push_back(def + "Object.xml");
		lc.mAdapters.push_back(def + "Adapter.xml");
		lc.mConcepts.push_back(def + "Concept.xml");
		lc.mProperties.push_back(def + "Value.xml");

		lc.mModules.push_back(level + "Object.xml");
		lc.mAdapters.push_back(level + "Adapter.xml");
		lc.mConcepts.push_back(level + "Concept.xml");
		lc.mProperties.push_back(level + "Value.xml");

		using BFG::Property::ValueId;

		BFG::PluginId spId = ValueId::ENGINE_PLUGIN_ID;
		BFG::PluginId ppId = BFG::Property::generatePluginId<BFG::PluginId>();

		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
		boost::shared_ptr<PongPlugin> pp(new PongPlugin(ppId));
		mPluginMap.insert(sp);
		mPluginMap.insert(pp);

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
	}

	virtual void destroyObject(GameHandle handle)
	{
		mSector->removeObject(handle);
	}

protected:
	BFG::Event::SubLanePtr mSubLane;
	GameHandle mStateHandle;
	GameHandle mPlayer1;
	GameHandle mPlayer2;
	boost::shared_ptr<BFG::Environment> mEnvironment;

private:
	BFG::Property::PluginMapT mPluginMap;
	boost::shared_ptr<BFG::Sector> mSector;
	boost::shared_ptr<BFG::GameObjectFactory> mGof;
};

struct ServerState: public SynchronizationTestState
{
	typedef std::map<BFG::Network::PeerIdT, BFG::GameHandle> ClientListT;

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

	void onReceived(const BFG::Network::DataPayload& payload, GameHandle peerId)
	{
		switch(payload.mAppEventId)
		{
		case A_SHIP_AXIS_Y:
		{
			GameHandle playerHandle = getPlayerHandle(peerId);
			if (playerHandle == BFG::NULL_HANDLE)
				return;
			f32 data;
			arrayToValue(data, payload.mAppData, 0);
			dbglog << "Server received A_SHIP_AXIS_Y (" << data << ")";
			mSubLane->emit(BFG::ID::GOE_CONTROL_YAW, data, playerHandle);
			break;
		}
		}
	}

	void createScene()
	{
		std::stringstream handles;

		BFG::ObjectParameter op;
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "Ball";
		op.mType = "PongBall";
		op.mLocation = v3(0.0f, 0.0f, OBJECT_Z_POSITION);
		op.mLinearVelocity = v3(0.0f, -15.0f, 0.0f);
		handles << op.mHandle << " ";

		createObject(op);
		mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

		op = BFG::ObjectParameter();
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "LowerBar";
		op.mType = "PongBar";
		op.mLocation = v3(0.0f, -BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		handles << op.mHandle << " ";
		mPlayer1 = op.mHandle;

		createObject(op);
		mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

		op = BFG::ObjectParameter();
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "UpperBar";
		op.mType = "PongBar";
		op.mLocation.position = v3(0.0f, BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		op.mLocation.orientation = BFG::qv4::IDENTITY;
		BFG::fromAngleAxis(op.mLocation.orientation, 180 * DEG2RAD, BFG::v3::UNIT_Z);
		handles << op.mHandle;
		mPlayer2 = op.mHandle;

		createObject(op);
		mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

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
		
		mCreatedHandles = "";
		mSceneCreated = false;
	}

	GameHandle getPlayerHandle(BFG::Network::PeerIdT peerId)
	{
		ClientListT::iterator it = mClientList.find(peerId);

		if (it == mClientList.end())
			return BFG::NULL_HANDLE;

		return it->second;
	}

	void onConnected(BFG::Network::PeerIdT peerId)
	{
		dbglog << "Client (" << peerId << ") wants to connect";
		ClientListT::iterator it = mClientList.find(peerId);

		if (it != mClientList.end())
		{
			errlog << "Client with the same PeerID (" << peerId << ") already connected!";
			return;
		}
		
		if (!mSceneCreated)
		{
			createScene();
		}

		CharArray512T ca512 = stringToArray<512>(mCreatedHandles);

		BFG::Network::DataPayload payload
		(
			CREATE_SCENE, 
			CLIENT_STATE_HANDLE, 
			SERVER_STATE_HANDLE,
			mCreatedHandles.length(),
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload, peerId);

		if (mClientList.size() < 1)
		{
			mClientList.insert(std::make_pair(peerId, mPlayer1));
		}
		else if (mClientList.size() == 1)
		{
			mClientList.insert(std::make_pair(peerId, mPlayer2));
		}

	}

	void onDisconnected(BFG::Network::PeerIdT peerId)
	{
		ClientListT::iterator it = mClientList.find(peerId);

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
		mSubLane->connect(A_QUIT, this, &ClientState::onQuit);
		mSubLane->connect(A_SHIP_AXIS_Y, this, &ClientState::onAxisY);
		mSubLane->connect(A_FPS, this, &ClientState::onFps);
		mSubLane->connect(A_CONSOLE, this, &ClientState::onConsole);
		mSubLane->connect(BFG::ID::NE_RECEIVED, this, &ClientState::onReceived, CLIENT_STATE_HANDLE);
		
		initController(CLIENT_STATE_HANDLE, lane);
	}

	virtual ~ClientState()
	{
		infolog << "Pong: Destroying GameState.";
	}

	void onQuit(s32)
	{
		mSubLane->emit(BFG::ID::EA_FINISH, BFG::Event::Void());
	}
	
	void onCreateScene(const BFG::Network::DataPayload& payload)
	{
		std::stringstream oss(payload.mAppData.data());

		BFG::ObjectParameter op;
		oss >> op.mHandle;
		op.mName = "Ball";
		op.mType = "PongBall";
		op.mLocation = v3(0.0f, 0.0f, OBJECT_Z_POSITION);

		createObject(op);
		mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

		op = BFG::ObjectParameter();
		oss >> op.mHandle;
		op.mName = "LowerBar";
		op.mType = "PongBar";
		op.mLocation = v3(0.0f, -BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);

		createObject(op);
		mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

		op = BFG::ObjectParameter();
		oss >> op.mHandle;
		op.mName = "UpperBar";
		op.mType = "PongBar";
		op.mLocation.position = v3(0.0f, BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		op.mLocation.orientation = BFG::qv4::IDENTITY;
		BFG::fromAngleAxis(op.mLocation.orientation, 180 * DEG2RAD, BFG::v3::UNIT_Z);

		createObject(op);
		mSubLane->emit(BFG::ID::GOE_SYNCHRONIZATION_MODE, BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);
	}

	void onAxisY(BFG::f32 axisValue)
	{
		CharArray512T ca512 = CharArray512T();
		valueToArray(BFG::clamp(axisValue, -1.0f, 1.0f), ca512, 0);
		BFG::Network::DataPayload payload
		(
			A_SHIP_AXIS_Y, 
			SERVER_STATE_HANDLE, 
			CLIENT_STATE_HANDLE,
			sizeof(f32),
			ca512
		);

		mSubLane->emit(BFG::ID::NE_SEND, payload);
	}

	void onFps(bool show)
	{
		mSubLane->emit(BFG::ID::VE_DEBUG_FPS, show);
	}

	void onConsole(bool show)
	{
		mSubLane->emit(BFG::ID::VE_CONSOLE, show);
	}

	void onReceived(const BFG::Network::DataPayload& payload)
	{
		switch(payload.mAppEventId)
		{
		case CREATE_SCENE:
		{
			onCreateScene(payload);
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
		  dbglog << "Pong: Creating ViewState.";
		  lane.emit(BFG::ID::VE_SET_AMBIENT, BFG::cv4(1.0f, 1.0f, 1.0f), handle);
	  }

	  virtual ~ViewState()
	  {
		  dbglog << "Pong: Destroying ViewState.";
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
		std::cerr << "For Server use: bfgPong <Port>\nFor Client use: bfgPong <IP> <Port>\n";
		BFG::Base::pause();
		return 0;
	}	

	//BFG::Base::Logger::SeverityLevel level = BFG::Base::Logger::SL_DEBUG;
	BFG::Base::Logger::SeverityLevel level = BFG::Base::Logger::SL_ERROR;

	if (server)
	{
		BFG::Path p;
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/bfgPongServer.log");

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
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/bfgPongClient.log");

		size_t controllerFrequency = 1000;

		BFG::Event::Synchronizer synchronizer;
		BFG::Event::Lane networkLane(synchronizer, 100, "Network");
		BFG::Event::Lane physicsLane(synchronizer, 100, "Physics", BFG::Event::RL2);
		BFG::Event::Lane viewLane(synchronizer, 100, "View");
		BFG::Event::Lane controllerLane(synchronizer, controllerFrequency, "Controller");
		BFG::Event::Lane clientLane(synchronizer, 100, "Client", BFG::Event::RL3);

		networkLane.addEntry<BFG::Network::Main>(BFG_CLIENT);
		physicsLane.addEntry<BFG::Physics::Main>();
		viewLane.addEntry<BFG::View::Main>("Pong");
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
