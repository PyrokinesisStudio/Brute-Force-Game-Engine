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

#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Base/ResolveDns.h>
#include <Base/Pause.h>
#include <Controller/Action.h>
#include <Controller/StateInsertion.h>
#include <Core/CharArray.h>
#include <Core/Path.h>
#include <Base/ShowException.h>
#include <Core/Types.h>
#include <Core/Utils.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>
#include <EventSystem/Event_fwd.h>
#include <Network/Network.h>
#include <View/HudElement.h>
#include <View/View.h>

#define BFG_USE_CONTROLLER
#define BFG_USE_NETWORK
#define BFG_USE_VIEW
#include <EngineInit.h>

using namespace BFG;

#define CHAT_MESSAGE 10000

typedef Event<EventIdT, CharArray512T, GameHandle, GameHandle> ChatEvent;

struct Server : Emitter
{
	Server(EventLoop* loop) :
	Emitter(loop),
	mHandle(123)
	{
		loop->connect(ID::NE_CONNECTED, this, &Server::netControlHandler);
		loop->connect(ID::NE_DISCONNECTED, this, &Server::netControlHandler);
		loop->connect(ID::NE_RECEIVED, this, &Server::netPacketHandler, mHandle);
	}
	
	~Server()
	{
		loop()->disconnect(ID::NE_CONNECTED, this);
		loop()->disconnect(ID::NE_DISCONNECTED, this);
		loop()->disconnect(ID::NE_RECEIVED, this);
	}
	
	void netControlHandler(Network::ControlEvent* e)
	{
		dbglog << "Chat::Server::netControlHandler: " << ID::asStr(static_cast<ID::NetworkAction>(e->getId()));
		
		switch (e->id())
		{
		case ID::NE_CONNECTED:
		{
			Network::PeerIdT peerId = boost::get<Network::PeerIdT>(e->getData());
			dbglog << "Chat::Server: Adding " <<  peerId << " to list.";
			peers.push_back(peerId);
			break;
		}
		case ID::NE_DISCONNECTED:
		{
			Network::PeerIdT peerId = boost::get<Network::PeerIdT>(e->getData());
			dbglog << "Chat::Server: Removing " << peerId << " from list.";
			std::vector<Network::PeerIdT>::iterator it = std::find(peers.begin(), peers.end(), peerId);
			if (it != peers.end())
				peers.erase(it);

		}
		default:
		{
			warnlog << "Server::netControlHandler: Got unhandled event: " << e->id();
			break;
		}
		} // switch
	}
	
	void netPacketHandler(Network::DataPacketEvent* e)
	{
		dbglog << "Chat::Server::netPacketHandler: " << ID::asStr(static_cast<ID::NetworkAction>(e->getId()));

		Network::DataPayload& payload = e->getData();
		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		
		dbglog << Network::debug(*e);
		
		payload.mAppDestination = payload.mAppSender;
		payload.mAppSender = mHandle;
		
		dbglog << "Chat::Server: Sending NOT to " << e->sender();
		for (size_t i=0; i<peers.size(); ++i)
		{
			if (peers[i] != e->sender())
			{
				dbglog << "Chat::Server: Sending to " << peers[i];
				emit<Network::DataPacketEvent>(ID::NE_SEND, payload, peers[i], 0);
			}
		}
// 		dbglog << "Chat::Server: Sending broadcast.";
// 		payload.mAppDataLen = 10;
// 		memcpy(payload.mAppData.data(), "Broadcast", payload.mAppDataLen);
// 		emit<Network::DataPacketEvent>(ID::NE_SEND, payload);
	}
	
	std::vector<Network::PeerIdT> peers;
	GameHandle mHandle;
};

class ChatWindow : public View::HudElement
{
public:
	ChatWindow(EventLoop* loop) :
	View::HudElement("ChatWindow.layout", "ChatWindow"),
	mControllerAdapter(generateHandle(), loop)
	{
		BFG::Controller_::ActionMapT actions;
		BFG::Controller_::fillWithDefaultActions(actions);
		BFG::Controller_::sendActionsToController(loop, actions);
	
		BFG::Path path;
		const std::string config_path = path.Expand("MyGUI.xml");
		const std::string state_name = "Chat";

		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);

		BFG::Controller_::StateInsertion si(config_path, state_name, generateHandle(), true, wa);

		BFG::EventFactory::Create<BFG::Controller_::ControlEvent>
		(
			loop,
			BFG::ID::CE_LOAD_STATE,
			si
		);
	}

private:
	virtual void internalUpdate(f32 time)
	{}

	View::ControllerMyGuiAdapter mControllerAdapter;
};

struct Client : Emitter
{
	Client(EventLoop* loop) :
	Emitter(loop),
	mHandle(456)
	{
		BFG::Controller_::ActionMapT actions;
		BFG::Controller_::fillWithDefaultActions(actions);
		BFG::Controller_::sendActionsToController(loop, actions);

		loop->connect(ID::NE_CONNECTED, this, &Client::netControlHandler);
		loop->connect(ID::NE_DISCONNECTED, this, &Client::netControlHandler);
		loop->connect(ID::NE_RECEIVED, this, &Client::netPacketHandler, mHandle);
	}
	
	~Client()
	{
		loop()->disconnect(ID::NE_CONNECTED, this);
		loop()->disconnect(ID::NE_DISCONNECTED, this);
		loop()->disconnect(ID::NE_RECEIVED, this);
	}

	void netControlHandler(Network::ControlEvent* e)
	{
		dbglog << "Chat::Client::netControlHandler: " << ID::asStr(static_cast<ID::NetworkAction>(e->getId()));
		
		switch(e->id())
		{
		case ID::NE_CONNECTED:
		{
			mChatWindow.reset(new ChatWindow(loop()));
			MyGUI::Gui& gui = MyGUI::Gui::getInstance();
			mChatOutput = gui.findWidget<MyGUI::EditBox>("chatOutput");
			mChatInput = gui.findWidget<MyGUI::EditBox>("chatInput");

			mChatInput->eventEditSelectAccept += newDelegate(this, &Client::onTextEntered);

			break;
		}
		case ID::NE_DISCONNECTED:
		{
			dbglog << "Connection to Server was lost.";
			break;
		}
		default:
			warnlog << "Client::netControlHandler: Got unhandled event: " << e->id();
		}
	}

	void netPacketHandler(Network::DataPacketEvent* e)
	{
		dbglog << "Chat::Client::netPacketHandler: " << ID::asStr(static_cast<ID::NetworkAction>(e->getId()));

		const Network::DataPayload& payload = e->getData();
		std::string msg(payload.mAppData.data(), payload.mAppDataLen);

		dbglog << Network::debug(*e);
		dbglog << "Chat::Client: " << msg;
		std::string oldChat(mChatOutput->getCaption());
		mChatOutput->setCaption(oldChat + "\n" + msg);
	}

	void onTextEntered(MyGUI::EditBox* sender)
	{
		std::string msg = mChatInput->getCaption();
		CharArray512T data = stringToArray<512>(msg);

		Network::DataPayload payload(CHAT_MESSAGE, 123, mHandle, msg.length(), data);
		emit<Network::DataPacketEvent>(ID::NE_SEND, payload);

		sender->setCaption("");
	}

	GameHandle mHandle;
	boost::scoped_ptr<ChatWindow> mChatWindow;
	MyGUI::EditBox* mChatOutput;
	MyGUI::EditBox* mChatInput;
};

void serverInitHandler(EventLoop& loop)
{
	Server s(&loop);
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	Base::pause();
	loop.stop();
}

void clientInitHandler(EventLoop& loop)
{
	Client c(&loop);
	Base::pause();
	loop.stop();
}

int main(int argc, const char* argv[]) try
{
	BFG::Configuration cfg("bfgChat");
	if  (argc == 2)
	{
		cfg.handler = boost::bind(&serverInitHandler, _1);
		cfg.port = argv[1];
	}
	else if (argc == 3)
	{
		cfg.handler = boost::bind(&clientInitHandler, _1);
		cfg.ip = argv[1];
		cfg.port = argv[2];
	}
	else
	{
		std::cerr << "For Server use: bfgChat <Port>\n"
			"For Client use: bfgChat <IP> <Port>\n";
		BFG::Base::pause();
		return 0;
	}
	
	BFG::Base::MainContainerT mains = BFG::engineInit(cfg);

	// Give EventSystem some time to stop all loops
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));
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
