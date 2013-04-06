/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#define BOOST_TEST_MODULE NetworkTest2
#include <boost/test/unit_test.hpp>

#include <fstream>
#include "Requisites.h"

const BFG::GameHandle serverAppHandle = 123;
const BFG::GameHandle clientAppHandle = 456;

const EventIdT testAppIdTcp = 10000;
const EventIdT testAppIdUdp = 10001;

const std::string testMsg = "Test 1234";
const CharArray512T testData = stringToArray<512>(testMsg);

struct Server : BFG::Emitter
{
	Server(EventLoop* loop, EventStatus& status) :
	BFG::Emitter(loop),
	mStatus(status)
	{
		BOOST_TEST_MESSAGE( "Server Constructor" );
		loop->connect(BFG::ID::NE_CONNECTED, this, &Server::netConnectHandler);
		loop->connect(BFG::ID::NE_DISCONNECTED, this, &Server::netDisconnectHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &Server::netPacketHandler, serverAppHandle);
	}
	
	~Server()
	{
		BOOST_TEST_MESSAGE( "Server Destructor" );
		loop()->disconnect(BFG::ID::NE_CONNECTED, this);
		loop()->disconnect(BFG::ID::NE_DISCONNECTED, this);
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
	}
	
	void netConnectHandler(BFG::Network::ControlEvent* e)
	{
		mStatus.gotConnected = true;
		BOOST_CHECK_EQUAL(e->id(), BFG::ID::NE_CONNECTED);
		BOOST_CHECK_NE(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}
	
	void netDisconnectHandler(BFG::Network::ControlEvent* e)
	{
		mStatus.gotDisconnected = true;
		BOOST_CHECK_EQUAL(e->id(), BFG::ID::NE_DISCONNECTED);
		BOOST_CHECK_NE(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}

	void netPacketHandler(BFG::Network::DataPacketEvent* e)
	{
		mStatus.gotReceived = true;
		BOOST_CHECK_EQUAL(e->id(), BFG::ID::NE_RECEIVED);
		BOOST_CHECK_EQUAL(e->destination(), serverAppHandle);
		BOOST_CHECK_NE(e->sender(), 0);
		
		const BFG::Network::DataPayload& payload = e->getData();

		EventIdT appId = payload.mAppEventId;

		dbglog << "TestServer::netPacketHandler got appId: " << appId;
		BOOST_CHECK (appId == testAppIdTcp || appId == testAppIdUdp);

		if (appId == testAppIdTcp)
			mStatus.gotTcpData = true;

		if (appId == testAppIdUdp)
			mStatus.gotUdpData = true;

		BFG::GameHandle destinationHandle = payload.mAppDestination;
		BFG::GameHandle senderHandle = payload.mAppSender;
		
		BOOST_CHECK_EQUAL (destinationHandle, serverAppHandle);
		BOOST_CHECK_EQUAL (senderHandle, clientAppHandle);
		
		BFG::u16 packetSize = payload.mAppDataLen;
		CharArray512T data = payload.mAppData;
		
		BOOST_CHECK_EQUAL (packetSize, testMsg.size());
		BOOST_CHECK_EQUAL_COLLECTIONS (data.begin(), data.begin()+packetSize, testData.begin(), testData.begin() + testMsg.size());
	}
	
	EventStatus& mStatus;
};

struct Client : BFG::Emitter
{
	Client(EventLoop* loop, EventStatus& status) :
	Emitter(loop),
	mStatus(status)
	{
		BOOST_TEST_MESSAGE( "Client Constructor" );
		loop->connect(BFG::ID::NE_CONNECTED, this, &Client::netConnectHandler);
		loop->connect(BFG::ID::NE_DISCONNECTED, this, &Client::netDisconnectHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &Client::netPacketHandler, clientAppHandle);
	}
	
	~Client()
	{
		BOOST_TEST_MESSAGE( "Client Destructor" );
		loop()->disconnect(BFG::ID::NE_CONNECTED, this);
		loop()->disconnect(BFG::ID::NE_DISCONNECTED, this);
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
	}

	void netConnectHandler(BFG::Network::ControlEvent* e)
	{
		mStatus.gotConnected = true;
		BOOST_CHECK_EQUAL(e->id(), BFG::ID::NE_CONNECTED);
		BOOST_CHECK_NE(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}

	void netDisconnectHandler(BFG::Network::ControlEvent* e)
	{
		mStatus.gotDisconnected = true;
		BOOST_CHECK_EQUAL(e->id(), BFG::ID::NE_DISCONNECTED);
		BOOST_CHECK_EQUAL(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}

	void netPacketHandler(BFG::Network::DataPacketEvent* e)
	{
		mStatus.gotReceived = true;
		BOOST_CHECK_EQUAL(e->id(), BFG::ID::NE_RECEIVED);
		BOOST_CHECK_EQUAL(e->destination(), clientAppHandle);
		BOOST_CHECK_EQUAL(e->sender(), 0);
		
		const BFG::Network::DataPayload& payload = e->getData();

		EventIdT appId = payload.mAppEventId;

		dbglog << "TestClient::netPacketHandler got appId: " << appId;
		BOOST_CHECK (appId == testAppIdTcp || appId == testAppIdUdp);
		
		if (appId == testAppIdTcp)
			mStatus.gotTcpData = true;

		if (appId == testAppIdUdp)
			mStatus.gotUdpData = true;
		
		BFG::GameHandle destinationHandle = payload.mAppDestination;
		BFG::GameHandle senderHandle = payload.mAppSender;
		
		BOOST_CHECK_EQUAL (destinationHandle, clientAppHandle);
		BOOST_CHECK_EQUAL (senderHandle, serverAppHandle);
		
		BFG::u16 packetSize = payload.mAppDataLen;
		CharArray512T data = payload.mAppData;
		
		BOOST_CHECK_EQUAL (packetSize, testMsg.size());
		BOOST_CHECK_EQUAL_COLLECTIONS (data.begin(), data.begin()+packetSize, testData.begin(), testData.begin() + testMsg.size());
	}
	
	EventStatus& mStatus;
};


typedef NetworkContext<Server, BFG_SERVER> ServerContext;
typedef NetworkContext<Client, BFG_CLIENT> ClientContext;

boost::scoped_ptr<ServerContext> server;
boost::scoped_ptr<ClientContext> client;

void resetEventStatus()
{
	server->status.reset();
	client->status.reset();
}

struct EventSystemSetup
{
	EventSystemSetup() :
	test_log("NetworkTest2.log")
	{
// 		boost::unit_test::unit_test_log.set_stream(test_log);
		BOOST_TEST_MESSAGE( "EventSystemSetup Constructor" );
// 		BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Test.log");
		BFG::Base::Logger::Init(BFG::Base::Logger::SL_INFORMATION, "Test.log");

		server.reset(new ServerContext("Server Loop"));
		client.reset(new ClientContext("Client Loop"));
	}

	~EventSystemSetup()
	{
		client.reset();
		server.reset();
		BOOST_TEST_MESSAGE( "EventSystemSetup Destructor" );
		boost::unit_test::unit_test_log.set_stream(std::cout);
	}

	std::ofstream test_log;
};

//! Generates a port as u16 and as std::string
static void generateRandomPort(BFG::u16& port, std::string& portString)
{
	srand(time(NULL));

	// Use something between 20000 and 30000
	port = 20000 + rand()%10000;
	
	srand(0);

	// Convert port u16 to string
	std::stringstream ss;
	ss << port;
	portString = ss.str();
}

BOOST_GLOBAL_FIXTURE (EventSystemSetup);

BOOST_AUTO_TEST_CASE (ConnectionTest)
{
	BOOST_TEST_MESSAGE( "ConnectionTest is starting" );
	resetEventStatus();

	BFG::u16 port;
	std::string portString;
	generateRandomPort(port, portString);
	
	server->emitter->emit<BFG::Network::ControlEvent>(BFG::ID::NE_LISTEN, static_cast<BFG::u16>(port));

	client->emitter->emit<BFG::Network::ControlEvent>
	(
		BFG::ID::NE_CONNECT,
		boost::make_tuple(stringToArray<128>("127.0.0.1"), stringToArray<128>(portString))
	);
	
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	BOOST_CHECK(server->status.gotConnected);
	BOOST_CHECK(client->status.gotConnected);
	
	BOOST_TEST_MESSAGE( "ConnectionTest has ended" );
}

BOOST_AUTO_TEST_CASE (ClientToServerDataCheck)
{
	BOOST_TEST_MESSAGE( "ClientToServerDataCheck is starting" );
	resetEventStatus();

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, serverAppHandle, clientAppHandle, testMsg.length(), testData);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, serverAppHandle, clientAppHandle, testMsg.length(), testData);
	client->emitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payloadTcp);
	client->emitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND_UDP, payloadUdp);
	
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	BOOST_CHECK(server->status.gotReceived);
	BOOST_CHECK(server->status.gotTcpData);
	BOOST_CHECK(server->status.gotUdpData);
	BOOST_CHECK(!client->status.gotReceived);
	BOOST_CHECK(!client->status.gotTcpData);
	BOOST_CHECK(!client->status.gotUdpData);

	BOOST_TEST_MESSAGE( "ClientToServerDataCheck has ended" );
}

BOOST_AUTO_TEST_CASE (ClientToServerDestinationNotNullCheck)
{
	BOOST_TEST_MESSAGE( "ClientToServerDestinationNotNullCheck is starting" );
	resetEventStatus();
	
	BFG::GameHandle bogusDestination = 123456789;

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, serverAppHandle, clientAppHandle, testMsg.length(), testData);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, serverAppHandle, clientAppHandle, testMsg.length(), testData);
	client->emitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payloadTcp, bogusDestination);
	client->emitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND_UDP, payloadUdp, bogusDestination);

	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	// DestinationId for NE_SEND on client side must always be 0.
	// Nobody should receive anything when an id is passed.
	BOOST_CHECK(!server->status.gotConnected);
	BOOST_CHECK(!server->status.gotDisconnected);
	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!client->status.gotConnected);
	BOOST_CHECK(!client->status.gotDisconnected);
	BOOST_CHECK(!client->status.gotReceived);

	BOOST_TEST_MESSAGE( "ClientToServerDestinationNotNullCheck has ended" );
}

BOOST_AUTO_TEST_CASE (ServerToClientDataCheck)
{
	BOOST_TEST_MESSAGE( "ServerToClientDataCheck is starting" );
	resetEventStatus();

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, clientAppHandle, serverAppHandle, testMsg.length(), testData);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, clientAppHandle, serverAppHandle, testMsg.length(), testData);
	server->emitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payloadTcp);
	server->emitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND_UDP, payloadUdp);
	
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!server->status.gotTcpData);
	BOOST_CHECK(!server->status.gotUdpData);
	BOOST_CHECK(client->status.gotReceived);
	BOOST_CHECK(client->status.gotTcpData);
	BOOST_CHECK(client->status.gotUdpData);
	
	BOOST_TEST_MESSAGE( "ServerToClientDataCheck has ended" );
}

BOOST_AUTO_TEST_CASE (ClientDisconnect)
{
	BOOST_TEST_MESSAGE( "ClientDisconnect is starting" );
	resetEventStatus();
	client->emitter->emit<BFG::Network::ControlEvent>(BFG::ID::NE_DISCONNECT, static_cast<BFG::u16>(0));

	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	BOOST_CHECK(server->status.gotDisconnected);
	BOOST_CHECK(client->status.gotDisconnected);
	
	BOOST_TEST_MESSAGE( "ClientDisconnect has ended" );
}

BOOST_AUTO_TEST_CASE (ServerShutdown)
{
	BOOST_TEST_MESSAGE( "ServerShutdown is starting" );
	resetEventStatus();
	server->emitter->emit<BFG::Network::ControlEvent>(BFG::ID::NE_SHUTDOWN, static_cast<BFG::u16>(0));

	boost::this_thread::sleep(boost::posix_time::seconds(1));
	
	// Should not receive any events since DISCONNECT was sent previously
	BOOST_CHECK(!server->status.gotConnected);
	BOOST_CHECK(!server->status.gotDisconnected);
	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!client->status.gotConnected);
	BOOST_CHECK(!client->status.gotDisconnected);
	BOOST_CHECK(!client->status.gotReceived);

	BOOST_TEST_MESSAGE( "ServerShutdown has ended" );
}
