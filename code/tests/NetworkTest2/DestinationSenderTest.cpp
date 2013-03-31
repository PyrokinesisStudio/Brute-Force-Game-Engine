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

#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>

#include <Network/Network.h>

#define BOOST_TEST_MODULE NetworkTest2
#include <boost/test/unit_test.hpp>
#include <fstream>

const BFG::GameHandle serverApplicationHandle = 123;
const BFG::GameHandle clientApplicationHandle = 456;

const EventIdT testAppIdTcp = 10000;
const EventIdT testAppIdUdp = 10001;

const std::string testMsg = "Test 1234";
CharArray512T testData = stringToArray<512>(testMsg);

// Make sure events are arriving
bool serverGotConnected = false;
bool serverGotDisconnected = false;
bool serverGotReceived = false;
bool serverGotTcpData = false;
bool serverGotUdpData = false;
bool clientGotConnected = false;
bool clientGotDisconnected = false;
bool clientGotReceived = false;
bool clientGotTcpData = false;
bool clientGotUdpData = false;

void resetEventStatus()
{
	serverGotConnected = false;
	serverGotDisconnected = false;
	serverGotReceived = false;
	serverGotTcpData = false;
	serverGotUdpData = false;
	clientGotConnected = false;
	clientGotDisconnected = false;
	clientGotReceived = false;
	clientGotTcpData = false;
	clientGotUdpData = false;
}

struct Server : BFG::Emitter
{
	Server(EventLoop* loop) :
	BFG::Emitter(loop)
	{
		BOOST_TEST_MESSAGE( "Server Constructor" );
		loop->connect(BFG::ID::NE_CONNECTED, this, &Server::netConnectHandler);
		loop->connect(BFG::ID::NE_DISCONNECTED, this, &Server::netDisconnectHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &Server::netPacketHandler, serverApplicationHandle);
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
		serverGotConnected = true;
		BOOST_REQUIRE_EQUAL(e->getId(), BFG::ID::NE_CONNECTED);
		BOOST_REQUIRE_NE(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}
	
	void netDisconnectHandler(BFG::Network::ControlEvent* e)
	{
		serverGotDisconnected = true;
		BOOST_REQUIRE_EQUAL(e->getId(), BFG::ID::NE_DISCONNECTED);
		BOOST_REQUIRE_NE(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}

	void netPacketHandler(BFG::Network::DataPacketEvent* e)
	{
		serverGotReceived = true;
		BOOST_REQUIRE_EQUAL(e->getId(), BFG::ID::NE_RECEIVED);
		BOOST_REQUIRE_EQUAL(e->destination(), serverApplicationHandle);
		BOOST_REQUIRE_NE(e->sender(), 0);
		
		const BFG::Network::DataPayload& payload = e->getData();

		EventIdT appId = payload.mAppEventId;

		BOOST_REQUIRE (appId == testAppIdTcp || appId == testAppIdUdp);

		if (appId == testAppIdTcp)
			serverGotTcpData = true;

		if (appId == testAppIdUdp)
			serverGotUdpData = true;

		BFG::GameHandle destinationHandle = payload.mAppDestination;
		BFG::GameHandle senderHandle = payload.mAppSender;
		
		BOOST_REQUIRE_EQUAL (destinationHandle, serverApplicationHandle);
		BOOST_REQUIRE_EQUAL (senderHandle, clientApplicationHandle);
		
		BFG::u16 packetSize = payload.mAppDataLen;
		CharArray512T data = payload.mAppData;
		
		BOOST_REQUIRE_EQUAL (packetSize, testMsg.size());
		BOOST_REQUIRE_EQUAL_COLLECTIONS (data.begin(), data.begin()+packetSize, testData.begin(), testData.begin() + testMsg.size());
	}
};

struct Client : BFG::Emitter
{
	Client(EventLoop* loop) :
	Emitter(loop)
	{
		BOOST_TEST_MESSAGE( "Client Constructor" );
		loop->connect(BFG::ID::NE_CONNECTED, this, &Client::netConnectHandler);
		loop->connect(BFG::ID::NE_DISCONNECTED, this, &Client::netDisconnectHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &Client::netPacketHandler, clientApplicationHandle);
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
		clientGotConnected = true;
		BOOST_REQUIRE_EQUAL(e->getId(), BFG::ID::NE_CONNECTED);
		BOOST_REQUIRE_NE(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}

	void netDisconnectHandler(BFG::Network::ControlEvent* e)
	{
		clientGotDisconnected = true;
		BOOST_REQUIRE_EQUAL(e->getId(), BFG::ID::NE_DISCONNECTED);
		BOOST_REQUIRE_EQUAL(boost::get<BFG::Network::PeerIdT>(e->getData()), 0);
	}

	void netPacketHandler(BFG::Network::DataPacketEvent* e)
	{
		clientGotReceived = true;
		BOOST_REQUIRE_EQUAL(e->getId(), BFG::ID::NE_RECEIVED);
		BOOST_REQUIRE_EQUAL(e->destination(), clientApplicationHandle);
		BOOST_REQUIRE_EQUAL(e->sender(), 0);
		
		const BFG::Network::DataPayload& payload = e->getData();

		EventIdT appId = payload.mAppEventId;

		BOOST_REQUIRE (appId == testAppIdTcp || appId == testAppIdUdp);
		
		if (appId == testAppIdTcp)
			serverGotTcpData = true;

		if (appId == testAppIdUdp)
			serverGotUdpData = true;
		
		BFG::GameHandle destinationHandle = payload.mAppDestination;
		BFG::GameHandle senderHandle = payload.mAppSender;
		
		BOOST_REQUIRE_EQUAL (destinationHandle, clientApplicationHandle);
		BOOST_REQUIRE_EQUAL (senderHandle, serverApplicationHandle);
		
		BFG::u16 packetSize = payload.mAppDataLen;
		CharArray512T data = payload.mAppData;
		
		BOOST_REQUIRE_EQUAL (packetSize, testMsg.size());
		BOOST_REQUIRE_EQUAL_COLLECTIONS (data.begin(), data.begin()+packetSize, testData.begin(), testData.begin() + testMsg.size());
	}
};

boost::shared_ptr<BFG::Emitter> serverEmitter;
boost::shared_ptr<BFG::Emitter> clientEmitter;

struct EventSystemSetup
{
	boost::shared_ptr<EventLoop> serverLoop;
	boost::shared_ptr<EventLoop> clientLoop;
	boost::scoped_ptr<Server> s;
	boost::scoped_ptr<Client> c;
	boost::scoped_ptr<BFG::Base::IEntryPoint> serverEp;
	boost::scoped_ptr<BFG::Base::IEntryPoint> clientEp;
	
	EventSystemSetup() :
	test_log( "NetworkTest2.log" )
	{
// 		boost::unit_test::unit_test_log.set_stream( test_log );
		BOOST_TEST_MESSAGE( "EventSystemSetup Constructor" );
// 		BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Test.log");
		BFG::Base::Logger::Init(BFG::Base::Logger::SL_INFORMATION, "Test.log");
		serverLoop.reset(new EventLoop(false, new EventSystem::BoostThread<>("ServerLoop"), new EventSystem::NoCommunication()));
		clientLoop.reset(new EventLoop(false, new EventSystem::BoostThread<>("ClientLoop"), new EventSystem::NoCommunication()));
		serverEp.reset(BFG::Network::Interface::getEntryPoint(BFG_SERVER));
		clientEp.reset(BFG::Network::Interface::getEntryPoint(BFG_CLIENT));
		serverLoop->addEntryPoint(serverEp.get());
		clientLoop->addEntryPoint(clientEp.get());
		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
		s.reset(new Server(serverLoop.get()));
		c.reset(new Client(clientLoop.get()));
		serverLoop->run();
		clientLoop->run();
		serverEmitter.reset(new BFG::Emitter(serverLoop.get()));
		clientEmitter.reset(new BFG::Emitter(clientLoop.get()));
	}

	~EventSystemSetup()
	{
		BOOST_TEST_MESSAGE( "EventSystemSetup Destructor" );
		serverLoop->setExitFlag();
		clientLoop->setExitFlag();
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		s.reset();
		c.reset();
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		serverEp.reset();
		clientEp.reset();
		serverEmitter.reset();
		clientEmitter.reset();
		boost::this_thread::sleep(boost::posix_time::seconds(1));
		serverLoop.reset();
 		clientLoop.reset();
		boost::unit_test::unit_test_log.set_stream( std::cout );
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
	
	serverEmitter->emit<BFG::Network::ControlEvent>(BFG::ID::NE_LISTEN, static_cast<BFG::u16>(port));

	clientEmitter->emit<BFG::Network::ControlEvent>
	(
		BFG::ID::NE_CONNECT,
		boost::make_tuple(stringToArray<128>("127.0.0.1"), stringToArray<128>(portString))
	);
	
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	BOOST_REQUIRE(serverGotConnected);
	BOOST_REQUIRE(clientGotConnected);
	
	BOOST_TEST_MESSAGE( "ConnectionTest has ended" );
}

BOOST_AUTO_TEST_CASE (ClientToServerDataCheck)
{
	BOOST_TEST_MESSAGE( "ClientToServerDataCheck is starting" );
	resetEventStatus();

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, serverApplicationHandle, clientApplicationHandle, testMsg.length(), testData);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, serverApplicationHandle, clientApplicationHandle, testMsg.length(), testData);
	clientEmitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payloadTcp);
	clientEmitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND_UDP, payloadUdp);
	
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	BOOST_REQUIRE(serverGotReceived);
	BOOST_REQUIRE(serverGotTcpData);
	BOOST_REQUIRE(serverGotUdpData);
	BOOST_REQUIRE(!clientGotReceived);
	BOOST_REQUIRE(!clientGotTcpData);
	BOOST_REQUIRE(!clientGotUdpData);

	BOOST_TEST_MESSAGE( "ClientToServerDataCheck has ended" );
}

BOOST_AUTO_TEST_CASE (ClientToServerDestinationNotNullCheck)
{
	BOOST_TEST_MESSAGE( "ClientToServerDestinationNotNullCheck is starting" );
	resetEventStatus();
	
	BFG::GameHandle bogusDestination = 123456789;

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, serverApplicationHandle, clientApplicationHandle, testMsg.length(), testData);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, serverApplicationHandle, clientApplicationHandle, testMsg.length(), testData);
	clientEmitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payloadTcp, bogusDestination);
	clientEmitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND_UDP, payloadUdp, bogusDestination);

	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	// DestinationId for NE_SEND on client side must always be 0.
	// Nobody should receive anything when an id is passed.
	BOOST_REQUIRE(!serverGotConnected);
	BOOST_REQUIRE(!serverGotDisconnected);
	BOOST_REQUIRE(!serverGotReceived);
	BOOST_REQUIRE(!clientGotConnected);
	BOOST_REQUIRE(!clientGotDisconnected);
	BOOST_REQUIRE(!clientGotReceived);

	BOOST_TEST_MESSAGE( "ClientToServerDestinationNotNullCheck has ended" );
}

BOOST_AUTO_TEST_CASE (ServerToClientDataCheck)
{
	BOOST_TEST_MESSAGE( "ServerToClientDataCheck is starting" );
	resetEventStatus();

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, clientApplicationHandle, serverApplicationHandle, testMsg.length(), testData);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, clientApplicationHandle, serverApplicationHandle, testMsg.length(), testData);
	serverEmitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payloadTcp);
	serverEmitter->emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND_UDP, payloadUdp);
	
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	BOOST_REQUIRE(!serverGotReceived);
	BOOST_REQUIRE(!serverGotTcpData);
	BOOST_REQUIRE(!serverGotUdpData);
	BOOST_REQUIRE(clientGotReceived);
	BOOST_REQUIRE(clientGotTcpData);
	BOOST_REQUIRE(clientGotUdpData);
	
	BOOST_TEST_MESSAGE( "ServerToClientDataCheck has ended" );
}

BOOST_AUTO_TEST_CASE (ClientDisconnect)
{
	BOOST_TEST_MESSAGE( "ClientDisconnect is starting" );
	resetEventStatus();
	clientEmitter->emit<BFG::Network::ControlEvent>(BFG::ID::NE_DISCONNECT, static_cast<BFG::u16>(0));

	boost::this_thread::sleep(boost::posix_time::milliseconds(250));

	BOOST_REQUIRE(serverGotDisconnected);
	BOOST_REQUIRE(clientGotDisconnected);
	
	BOOST_TEST_MESSAGE( "ClientDisconnect has ended" );
}

BOOST_AUTO_TEST_CASE (ServerShutdown)
{
	BOOST_TEST_MESSAGE( "ServerShutdown is starting" );
	resetEventStatus();
	serverEmitter->emit<BFG::Network::ControlEvent>(BFG::ID::NE_SHUTDOWN, static_cast<BFG::u16>(0));

	boost::this_thread::sleep(boost::posix_time::seconds(1));
	
	// Should not receive any events since DISCONNECT was sent previously
	BOOST_REQUIRE(!serverGotConnected);
	BOOST_REQUIRE(!serverGotDisconnected);
	BOOST_REQUIRE(!serverGotReceived);
	BOOST_REQUIRE(!clientGotConnected);
	BOOST_REQUIRE(!clientGotDisconnected);
	BOOST_REQUIRE(!clientGotReceived);

	BOOST_TEST_MESSAGE( "ServerShutdown has ended" );
}
