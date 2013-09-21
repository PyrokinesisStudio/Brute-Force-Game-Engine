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

#include <MyGUI.h>

#include <boost/thread.hpp>

#include <Base/ShowException.h>
#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Base/ResolveDns.h>
#include <Base/Pause.h>

#include <Core/Types.h>
#include <Core/GameHandle.h>
#include <Core/Path.h>
#include <Core/CharArray.h>

#include <Event/Event.h>

#include <Controller/Action.h>
#include <Controller/StateInsertion.h>

#include <Network/Network.h>

#include <View/HudElement.h>
#include <View/View.h>

#define BFG_USE_CONTROLLER
#define BFG_USE_NETWORK
#define BFG_USE_VIEW
#include <EngineInit.h>

using namespace BFG;

#define CHAT_MESSAGE 10000

//typedef Event<EventIdT, CharArray512T, GameHandle, GameHandle> ChatEvent;

struct Server
{
	Server(BFG::Event::SubLanePtr sublane) :
	mHandle(123),
	mSubLane(sublane)
	{
		sublane->connect(ID::NE_CONNECTED, this, &Server::onConnected);
		sublane->connect(ID::NE_DISCONNECTED, this, &Server::onDisconnected);
		sublane->connect(ID::NE_RECEIVED, this, &Server::netPacketHandler, mHandle);
	}
	
	void onConnected(const Network::PeerIdT& peerId)
	{
		dbglog << "Chat::Server: Adding " <<  peerId << " to list.";
		peers.push_back(peerId);
	}

	void onDisconnected(const Network::PeerIdT& peerId)
	{
		dbglog << "Chat::Server: Removing " << peerId << " from list.";
		std::vector<Network::PeerIdT>::iterator it = std::find(peers.begin(), peers.end(), peerId);
		
		if (it != peers.end())
			peers.erase(it);
	}

	void netPacketHandler(const Network::DataPayload& payload, Event::SenderIdT sender)
	{
		dbglog << "Chat::Server::netPacketHandler: " << ID::asStr(static_cast<ID::NetworkAction>(ID::NE_RECEIVED));

		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		
		Network::DataPayload answer = payload;
		answer.mAppDestination = payload.mAppSender;
		answer.mAppSender = mHandle;
		
		dbglog << "Chat::Server: Sending NOT to " << sender;
		for (size_t i=0; i<peers.size(); ++i)
		{
			if (peers[i] != sender)
			{
				dbglog << "Chat::Server: Sending to " << peers[i];
				mSubLane->emit(ID::NE_SEND, answer, peers[i]);
			}
		}
// 		dbglog << "Chat::Server: Sending broadcast.";
// 		payload.mAppDataLen = 10;
// 		memcpy(payload.mAppData.data(), "Broadcast", payload.mAppDataLen);
// 		emit<Network::DataPacketEvent>(ID::NE_SEND, payload);
	}
	
	std::vector<Network::PeerIdT> peers;
	GameHandle mHandle;
	Event::SubLanePtr mSubLane;
};

class ChatWindow : public View::HudElement
{
public:
	ChatWindow(Event::Lane& lane) :
	View::HudElement("ChatWindow.layout", "ChatWindow"),
	mControllerAdapter(generateHandle(), lane)
	{
		BFG::Controller_::ActionMapT actions;
		BFG::Controller_::fillWithDefaultActions(actions);
		BFG::Controller_::sendActionsToController(lane, actions);
	
		BFG::Path path;
		const std::string config_path = path.Expand("MyGUI.xml");
		const std::string state_name = "Chat";

		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);
		BFG::Controller_::StateInsertion si(config_path, state_name, generateHandle(), true, wa);

		lane.emit(ID::CE_LOAD_STATE, si);
	}

private:
	virtual void internalUpdate(f32 time) {}

	View::ControllerMyGuiAdapter mControllerAdapter;
};

struct Client
{
	Client(Event::Lane& lane) :
		mHandle(456),
		mLane(lane)
	{
		BFG::Controller_::ActionMapT actions;
		BFG::Controller_::fillWithDefaultActions(actions);
		BFG::Controller_::sendActionsToController(lane, actions);

		lane.connect(ID::NE_CONNECTED, this, &Client::onConnected);
		lane.connect(ID::NE_DISCONNECTED, this, &Client::onDisconnected);
		lane.connect(ID::NE_RECEIVED, this, &Client::netPacketHandler, mHandle);
	}

	void onConnected(const Network::PeerIdT& peerId)
	{
		mChatWindow.reset(new ChatWindow(mLane));
		MyGUI::Gui& gui = MyGUI::Gui::getInstance();
		mChatOutput = gui.findWidget<MyGUI::EditBox>("chatOutput");
		mChatInput = gui.findWidget<MyGUI::EditBox>("chatInput");

		mChatInput->eventEditSelectAccept += newDelegate(this, &Client::onTextEntered);
	}

	void onDisconnected(const Network::PeerIdT& peerId)
	{
		dbglog << "Connection to Server was lost.";
	}
	
	void netPacketHandler(const Network::DataPayload& payload, Event::SenderIdT sender)
	{
		dbglog << "Chat::Client::netPacketHandler: " << ID::asStr(static_cast<ID::NetworkAction>(ID::NE_RECEIVED));

		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		dbglog << "Chat::Client: " << msg;
		std::string oldChat(mChatOutput->getCaption());
		mChatOutput->setCaption(oldChat + "\n" + msg);
	}

	void onTextEntered(MyGUI::EditBox* sender)
	{
		std::string msg = mChatInput->getCaption();
		CharArray512T data = stringToArray<512>(msg);

		Network::DataPayload payload(CHAT_MESSAGE, 123, mHandle, msg.length(), data);
		mLane.emit(ID::NE_SEND, payload);

		sender->setCaption("");
	}

	GameHandle mHandle;
	Event::Lane& mLane;
	boost::scoped_ptr<ChatWindow> mChatWindow;
	MyGUI::EditBox* mChatOutput;
	MyGUI::EditBox* mChatInput;
};

using namespace BFG;

int main(int argc, const char* argv[]) try
{
	// Fill configuration for initialization.
	
	Init::Configuration cfg("bfgChat");
	
	if  (argc == 2)
	{
		cfg.port = argv[1];
		cfg.runMode = Init::RM_SERVER;
	}
	else if (argc == 3)
	{
		cfg.ip = argv[1];
		cfg.port = argv[2];
		cfg.runMode = Init::RM_CLIENT;
	}
	else
	{
		std::cerr << "For Server use: bfgChat <Port>\n"
		          << "For Client use: bfgChat <IP> <Port>\n";
		Base::pause();
		return 0;
	}
	
	// Init engine modules and logger.
	Init::engine(cfg);

	// Init custom modules, states etc ...

	boost::scoped_ptr<Server> server;
	boost::scoped_ptr<Client> client;

	if (cfg.runMode == Init::RM_SERVER)
		server.reset(new Server(Init::gNetworkLane->createSubLane()));
	else if (cfg.runMode == Init::RM_CLIENT)
		client.reset(new Client(*Init::gNetworkLane));

	// Lets go!
	Init::startEngine(cfg);

	dbglog << "Good bye!";
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
