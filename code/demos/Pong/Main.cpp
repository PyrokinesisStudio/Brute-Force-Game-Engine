
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
#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Controller/Controller.h>
#include <Core/Math.h>
#include <Core/Path.h>
#include <Base/ShowException.h>
#include <Core/Utils.h>
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

struct SynchronizationTestState: BFG::State
{
	SynchronizationTestState(GameHandle handle, EventLoop* loop) :
	State(loop),
	mStateHandle(handle),
	mPlayer(NULL_HANDLE),
	mPlayer2(NULL_HANDLE),
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

		mGof.reset(new BFG::GameObjectFactory(this->loop(), lc, mPluginMap, mEnvironment, mStateHandle));

		mSector.reset(new BFG::Sector(this->loop(), 1, "Blah", mGof));
	}

	virtual ~SynchronizationTestState()
	{
	}

	virtual void onTick(const quantity<si::time, f32> TSLF)
	{
		mSector->update(TSLF);

		emit<BFG::Physics::Event>(BFG::ID::PE_STEP, TSLF.value());
	}

	virtual void createObject(const BFG::ObjectParameter& param)
	{
		boost::shared_ptr<BFG::GameObject> playerShip = mGof->createGameObject(param);
		mSector->addObject(playerShip);
		//emit<BFG::GameObjectEvent>(BFG::ID::GOE_GHOST_MODE, true, param.mHandle);
	}

	virtual void destroyObject(GameHandle handle)
	{
		mSector->removeObject(handle);
	}

protected:
	GameHandle mStateHandle;
	GameHandle mPlayer;
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

	ServerState(GameHandle handle, EventLoop* loop) :
	SynchronizationTestState(handle, loop),
	mSceneCreated(false)
	{
		loop->connect(BFG::ID::NE_RECEIVED, this, &ServerState::networkPacketEventHandler, SERVER_STATE_HANDLE);
		loop->connect(BFG::ID::NE_CONNECTED, this, &ServerState::networkControlEventHandler);
		loop->connect(BFG::ID::NE_DISCONNECTED, this, &ServerState::networkControlEventHandler);
	}
	
	virtual ~ServerState()
	{
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
		loop()->disconnect(BFG::ID::NE_CONNECTED, this);
		loop()->disconnect(BFG::ID::NE_DISCONNECTED, this);
	}

	void networkPacketEventHandler(BFG::Network::DataPacketEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_RECEIVED:
		{
			const BFG::Network::DataPayload& payload = e->getData();

			switch(payload.mAppEventId)
			{
			case A_SHIP_AXIS_Y:
			{
				GameHandle playerHandle = getPlayerHandle(e->sender());
				if (playerHandle == NULL_HANDLE)
					return;
				f32 data;
				arrayToValue(data, payload.mAppData, 0);
				dbglog << "Server received A_SHIP_AXIS_Y (" << data << ")";
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_CONTROL_YAW, data, playerHandle);
				break;
			}
			}
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
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

		op = BFG::ObjectParameter();
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "LowerBar";
		op.mType = "PongBar";
		op.mLocation = v3(0.0f, -BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		handles << op.mHandle << " ";
		mPlayer = op.mHandle;

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

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
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

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
			return NULL_HANDLE;

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

		emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload, peerId);

		if (mClientList.size() < 1)
		{
			mClientList.insert(std::make_pair(peerId, mPlayer));
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

	void networkControlEventHandler(BFG::Network::ControlEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_CONNECTED:
		{
			const BFG::Network::PeerIdT peerId = boost::get<BFG::Network::PeerIdT>(e->getData());
			onConnected(peerId);
			break;
		}
		case BFG::ID::NE_DISCONNECTED:
		{
			const BFG::Network::PeerIdT peerId = boost::get<BFG::Network::PeerIdT>(e->getData());
			onDisconnected(peerId);
			break;
		}
	}

	}

private:
	ClientListT mClientList;

	std::string mCreatedHandles;
	bool mSceneCreated;
};

struct ClientState : public SynchronizationTestState
{
	ClientState(GameHandle handle, EventLoop* loop) :
	SynchronizationTestState(handle, loop)
	{
		loop->connect(A_QUIT, this, &ClientState::controllerEventHandler);
		loop->connect(A_SHIP_AXIS_Y, this, &ClientState::controllerEventHandler);
		loop->connect(A_FPS, this, &ClientState::controllerEventHandler);
		loop->connect(A_CONSOLE, this, &ClientState::controllerEventHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &ClientState::networkEventHandler, CLIENT_STATE_HANDLE);
	}

	virtual ~ClientState()
	{
		infolog << "Tutorial: Destroying GameState.";
		loop()->disconnect(A_QUIT, this);
		loop()->disconnect(A_SHIP_AXIS_Y, this);
		loop()->disconnect(A_FPS, this);
		loop()->disconnect(A_CONSOLE, this);
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
	}

	void onQuit()
	{
		loop()->stop();
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
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

		op = BFG::ObjectParameter();
		oss >> op.mHandle;
		op.mName = "LowerBar";
		op.mType = "PongBar";
		op.mLocation = v3(0.0f, -BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

		op = BFG::ObjectParameter();
		oss >> op.mHandle;
		op.mName = "UpperBar";
		op.mType = "PongBar";
		op.mLocation.position = v3(0.0f, BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		op.mLocation.orientation = BFG::qv4::IDENTITY;
		BFG::fromAngleAxis(op.mLocation.orientation, 180 * DEG2RAD, BFG::v3::UNIT_Z);

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);
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
		emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);
	}

	void controllerEventHandler(BFG::Controller_::VipEvent* e)
	{
		switch(e->getId())
		{
		case A_QUIT:
			onQuit();
			break;
		case A_SHIP_AXIS_Y:
			onAxisY(boost::get<BFG::f32>(e->getData()));
			break;
		case A_FPS:
		{
			emit<BFG::View::Event>(BFG::ID::VE_DEBUG_FPS, boost::get<bool>(e->getData()));
			break;
		}
		case A_CONSOLE:
		{
			emit<BFG::View::Event>(BFG::ID::VE_CONSOLE, boost::get<bool>(e->getData()));
			break;
		}
		}
	}

	void networkEventHandler(BFG::Network::DataPacketEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_RECEIVED:
		{
			const BFG::Network::DataPayload& payload = e->getData();

			switch(payload.mAppEventId)
			{
			case CREATE_SCENE:
				onCreateScene(payload);
				break;
			}
		}
		}
	}
};

struct ViewState : public BFG::View::State
{
public:
	ViewState(GameHandle handle, EventLoop* loop) :
	State(handle, loop),
	mControllerMyGuiAdapter(handle, loop)
	{
		emit<BFG::View::Event>(BFG::ID::VE_SET_AMBIENT, BFG::cv4(1.0f, 1.0f, 1.0f), handle);
	}

	~ViewState()
	{
		infolog << "SynchronizationTest: Destroying ViewState.";
		emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
	}

	virtual void pause()
	{}

	virtual void resume()
	{}
	
private:
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

void initController(BFG::GameHandle stateHandle, EventLoop* loop)
{
	// The Emitter is the standard tool to send events with.
	BFG::Emitter emitter(loop);

	BFG::Controller_::ActionMapT actions;
	actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
	actions[A_FPS]         = "A_FPS";
	actions[A_QUIT]        = "A_QUIT";
	actions[A_CONSOLE]     = "A_CONSOLE";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(emitter.loop(), actions);

	BFG::Path path;
	const std::string configPath = path.Expand("Pong.xml");
	const std::string stateName = "Pong";

	BFG::View::WindowAttributes wa;
	BFG::View::queryWindowAttributes(wa);
	
	BFG::Controller_::StateInsertion si(configPath, stateName, stateHandle, true, wa);
	emitter.emit<BFG::Controller_::ControlEvent>
	(
		BFG::ID::CE_LOAD_STATE,
		si
	);
}

boost::scoped_ptr<ServerState> gServerState;
boost::scoped_ptr<ViewState> gViewState;
boost::scoped_ptr<ClientState> gClientState;

void* createServerState(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);

	gServerState.reset(new ServerState(SERVER_STATE_HANDLE, loop));

	return 0;
}

void* createClientStates(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);
	
	gViewState.reset(new ViewState(CLIENT_STATE_HANDLE, loop));
	gClientState.reset(new ClientState(CLIENT_STATE_HANDLE, loop));

	initController(CLIENT_STATE_HANDLE, loop);
	return 0;
}

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

	EventLoop loop1
	(
		true,
		new EventSystem::BoostThread<>("Loop1"),
		new EventSystem::NoCommunication()
	);

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

		BFG::Network::Main networkMain(&loop1, BFG_SERVER);
		BFG::Physics::Main physicsMain;
		loop1.addEntryPoint(networkMain.entryPoint());
		loop1.addEntryPoint(physicsMain.entryPoint());
		loop1.addEntryPoint(new BFG::Base::CEntryPoint(&createServerState));
		loop1.run();

		BFG::Emitter e(&loop1);
		e.emit<BFG::Network::ControlEvent>(BFG::ID::NE_LISTEN, port);

		BFG::Base::pause();

		loop1.stop();

		gServerState.reset();
	}
	else
	{
		std::string ip(argv[1]);
		std::string port(argv[2]);

		BFG::Path p;
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/bfgPongClient.log");

		size_t controllerFrequency = 50;

		BFG::Network::Main networkMain(&loop1, BFG_CLIENT);
		BFG::Physics::Main physicsMain;
		BFG::Controller_::Main controllerMain(controllerFrequency);
		BFG::View::Main viewMain("Pong");

		loop1.addEntryPoint(networkMain.entryPoint());
		loop1.addEntryPoint(physicsMain.entryPoint());
		loop1.addEntryPoint(viewMain.entryPoint());
		loop1.addEntryPoint(controllerMain.entryPoint());
		loop1.addEntryPoint(new BFG::Base::CEntryPoint(&createClientStates));
		
		loop1.run();

		BFG::Network::EndpointT payload = make_tuple(stringToArray<128>(ip), stringToArray<128>(port));

		BFG::Emitter e(&loop1);
		e.emit<BFG::Network::ControlEvent>(BFG::ID::NE_CONNECT, payload);

		while(!loop1.shouldExit())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}

		gViewState.reset();
		gClientState.reset();
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

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
