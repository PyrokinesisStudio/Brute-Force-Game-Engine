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

#include <fstream>
#include <boost/test/unit_test.hpp>

#include <Base/Logger.h>
#include <Base/PeriodicWaitForEqual.h>
#include <Base/TestAppDeco.h>

#include "Requisites.h"

using BFG::Base::periodicWaitForEqual;

const BFG::GameHandle serverAppHandle = 123;
const BFG::GameHandle client_1_AppHandle = 456;
const BFG::GameHandle client_2_AppHandle = 789;

const BFG::Event::IdT testAppIdTcp = 10000;
const BFG::Event::IdT testAppIdUdp = 10001;

const std::string testMsg1 = "Test 1234";
const CharArray512T testData1 = stringToArray<512>(testMsg1);

const std::string testMsg2 = "Test €#!§$%*'\"`^";
const CharArray512T testData2 = stringToArray<512>(testMsg2);

struct Server
{
	Server(BFG::Event::Lane& lane, EventStatus& status, BFG::GameHandle appHandle, const std::string& testMsg) :
		mLane(lane),
		mStatus(status),
		mAppHandle(appHandle),
		mTestMsg(testMsg)
	{
		BOOST_TEST_MESSAGE("Server Constructor");
		mLane.connect(BFG::ID::NE_CONNECTED, this, &Server::netConnectHandler);
		mLane.connect(BFG::ID::NE_DISCONNECTED, this, &Server::netDisconnectHandler);
		mLane.connect(BFG::ID::NE_RECEIVED, this, &Server::netPacketHandler, mAppHandle);
		mLane.connectV(BFG::ID::NE_SERVER_READY, this, &Server::netServerReadyHandler);
	}
	
	~Server()
	{
		BOOST_TEST_MESSAGE("Server Destructor");
	}
	
	void netConnectHandler(BFG::Network::PeerIdT pId)
	{
		mStatus.gotConnected = true;
		BOOST_CHECK_NE(pId, 0);
	}
	
	void netDisconnectHandler(BFG::Network::PeerIdT pId)
	{
		mStatus.gotDisconnected = true;
		BOOST_CHECK_NE(pId, 0);
	}

	void netPacketHandler(BFG::Network::DataPayload payload)
	{
		mStatus.gotReceived = true;
		
		BFG::Event::IdT appId = payload.mAppEventId;

		dbglog << "TestServer::netPacketHandler got appId: " << appId;
		BOOST_CHECK (appId == testAppIdTcp || appId == testAppIdUdp);

		if (appId == testAppIdTcp)
			mStatus.gotTcpData = true;

		if (appId == testAppIdUdp)
			mStatus.gotUdpData = true;

		BFG::GameHandle destinationHandle = payload.mAppDestination;
		BFG::GameHandle senderHandle = payload.mAppSender;
		
		BOOST_CHECK_EQUAL (destinationHandle, mAppHandle);
		BOOST_CHECK_EQUAL (senderHandle, client_1_AppHandle);
		
		BFG::u16 packetSize = payload.mAppDataLen;
		CharArray512T data = payload.mAppData;
		
		BOOST_CHECK_EQUAL (packetSize, mTestMsg.size());
		BOOST_CHECK_EQUAL_COLLECTIONS (data.begin(), data.begin()+packetSize, mTestMsg.begin(), mTestMsg.end());
	}
	
	void netServerReadyHandler()
	{
		mStatus.gotReady = true;
	}
	
	BFG::Event::Lane& mLane;
	EventStatus& mStatus;
	const BFG::GameHandle mAppHandle;
	const std::string& mTestMsg;
};

struct Client
{
	Client(BFG::Event::Lane& lane, EventStatus& status, BFG::GameHandle appHandle, const std::string& testMsg) :
		mLane(lane),
		mStatus(status),
		mAppHandle(appHandle),
		mTestMsg(testMsg)
	{
		std::stringstream ss;
		ss << "Client Construction (AppHandle: " << mAppHandle << ")";
		BOOST_TEST_MESSAGE(ss.str().c_str());
		
		mLane.connect(BFG::ID::NE_CONNECTED, this, &Client::netConnectHandler);
		mLane.connect(BFG::ID::NE_DISCONNECTED, this, &Client::netDisconnectHandler);
		mLane.connect(BFG::ID::NE_RECEIVED, this, &Client::netPacketHandler, mAppHandle);
	}
	
	virtual ~Client()
	{
		BOOST_TEST_MESSAGE("Client Destructor");
	}

	void netConnectHandler(BFG::Network::PeerIdT pId)
	{
		mStatus.gotConnected = true;
		BOOST_CHECK_NE(pId, 0);
	}

	void netDisconnectHandler(BFG::Network::PeerIdT pId)
	{
		mStatus.gotDisconnected = true;
		BOOST_CHECK_EQUAL(pId, 0);
	}

	void netPacketHandler(BFG::Network::DataPayload payload)
	{
		dbglog << "Client[" << mAppHandle << "]::netPacketHandler()";
		
		mStatus.gotReceived = true;
		
		BFG::Event::IdT appId = payload.mAppEventId;

		dbglog << "TestClient::netPacketHandler got appId: " << appId;
		BOOST_CHECK (appId == testAppIdTcp || appId == testAppIdUdp);
		
		if (appId == testAppIdTcp)
			mStatus.gotTcpData = true;

		if (appId == testAppIdUdp)
			mStatus.gotUdpData = true;
		
		BFG::GameHandle destinationHandle = payload.mAppDestination;
		BFG::GameHandle senderHandle = payload.mAppSender;
		
		BOOST_CHECK_EQUAL (destinationHandle, mAppHandle);
		BOOST_CHECK_EQUAL (senderHandle, serverAppHandle);
		
		BFG::u16 packetSize = payload.mAppDataLen;
		CharArray512T data = payload.mAppData;
		
		BOOST_CHECK_EQUAL (packetSize, mTestMsg.size());
		BOOST_CHECK_EQUAL_COLLECTIONS (data.begin(), data.begin()+packetSize, mTestMsg.begin(), mTestMsg.end());
	}
	
	BFG::Event::Lane& mLane;
	EventStatus& mStatus;
	const BFG::GameHandle mAppHandle;
	const std::string& mTestMsg;
};

typedef NetworkContext<Server, BFG_SERVER, serverAppHandle> ServerContext;
typedef NetworkContext<Client, BFG_CLIENT, client_1_AppHandle> Client1Context;
typedef NetworkContext<Client, BFG_CLIENT, client_2_AppHandle> Client2Context;

boost::scoped_ptr<ServerContext> server;
boost::scoped_ptr<Client1Context> client1;
boost::scoped_ptr<Client2Context> client2;

void resetEventStatus()
{
	server->status.reset();
	client1->status.reset();
	client2->status.reset();
}

struct EventSystemSetup
{
	EventSystemSetup() :
	test_log("NetworkTest2.log")
	{
// 		boost::unit_test::unit_test_log.set_stream(test_log);
		BOOST_TEST_MESSAGE("EventSystemSetup Constructor");
		BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Test.log");
//		BFG::Base::Logger::Init(BFG::Base::Logger::SL_INFORMATION, "Test.log");

		server.reset(new ServerContext("Server Loop", testMsg1));
		client1.reset(new Client1Context("Client Loop", testMsg1));
		client2.reset(new Client2Context("Client2 Loop", testMsg2));
	}

	~EventSystemSetup()
	{
		BOOST_TEST_MESSAGE("EventSystemSetup Destructor");
		client2.reset();
		client1.reset();
		server.reset();
		boost::unit_test::unit_test_log.set_stream(std::cout);
	}

	std::ofstream test_log;
};

//! Generates a port as u16 and as std::string
static void generateRandomPort(BFG::u16& port, std::string& portString)
{
	srand(static_cast<unsigned int>(time(NULL)));

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
	BFG::Base::Deco::h1("\nStarting ConnectionTest");
	resetEventStatus();

	BFG::u16 port;
	std::string portString;
	generateRandomPort(port, portString);
	
	server->lane.emit(BFG::ID::NE_LISTEN, static_cast<BFG::u16>(port));
	
	EventStatus serverExpected;
	serverExpected.gotReady = true;
	
	periodicWaitForEqual(serverExpected, server->status, boost::posix_time::milliseconds(5000));

	client1->lane.emit
	(
		BFG::ID::NE_CONNECT,
		boost::make_tuple(stringToArray<128>("127.0.0.1"), stringToArray<128>(portString))
	);

	client2->lane.emit
	(
		BFG::ID::NE_CONNECT,
		boost::make_tuple(stringToArray<128>("127.0.0.1"), stringToArray<128>(portString))
	);
	
	EventStatus expected;
	expected.gotConnected = true;
	
	periodicWaitForEqual(expected, client1->status, boost::posix_time::milliseconds(5000));
	periodicWaitForEqual(expected, client2->status, boost::posix_time::milliseconds(5000));

	BOOST_CHECK(client1->status.gotConnected);
	BOOST_CHECK(client2->status.gotConnected);
	BOOST_CHECK(server->status.gotConnected);
	
	BOOST_TEST_MESSAGE("ConnectionTest done.");
}

BOOST_AUTO_TEST_CASE (ClientToServerDataCheck)
{
	BFG::Base::Deco::h1("\nStarting ClientToServerDataCheck");
	resetEventStatus();

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, serverAppHandle, client_1_AppHandle, testMsg1.length(), testData1);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, serverAppHandle, client_1_AppHandle, testMsg1.length(), testData1);
	client1->lane.emit(BFG::ID::NE_SEND, payloadTcp);
	client1->lane.emit(BFG::ID::NE_SEND_UDP, payloadUdp);
	
	EventStatus s;
	s.gotReceived = true;
	s.gotTcpData  = true;
	s.gotUdpData  = true;
	periodicWaitForEqual(s, server->status, boost::posix_time::milliseconds(5000));
	
	BOOST_CHECK(server->status.gotReceived);
	BOOST_CHECK(server->status.gotTcpData);
	BOOST_CHECK(server->status.gotUdpData);
	BOOST_CHECK(!client1->status.gotReceived);
	BOOST_CHECK(!client1->status.gotTcpData);
	BOOST_CHECK(!client1->status.gotUdpData);
	BOOST_CHECK(!client2->status.gotReceived);
	BOOST_CHECK(!client2->status.gotTcpData);
	BOOST_CHECK(!client2->status.gotUdpData);

	BOOST_TEST_MESSAGE("ClientToServerDataCheck done");
}

BOOST_AUTO_TEST_CASE (ClientToServerDestinationNotNullCheck)
{
	BFG::Base::Deco::h1("\nStarting ClientToServerDestinationNotNullCheck");
	resetEventStatus();
	
	BFG::GameHandle bogusDestination = 123456789;

	BFG::Network::DataPayload payloadTcp(testAppIdTcp, serverAppHandle, client_1_AppHandle, testMsg1.length(), testData1);
	BFG::Network::DataPayload payloadUdp(testAppIdUdp, serverAppHandle, client_1_AppHandle, testMsg1.length(), testData1);
	client1->lane.emit(BFG::ID::NE_SEND, payloadTcp, bogusDestination);
	client1->lane.emit(BFG::ID::NE_SEND_UDP, payloadUdp, bogusDestination);

	// Must sleep since nothing will happen (best case).
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	
	BOOST_TEST_MESSAGE("No output expected.");

	// DestinationId for NE_SEND on client side must always be 0.
	// Nobody should receive anything when an id is passed.
	BOOST_CHECK(!server->status.gotConnected);
	BOOST_CHECK(!server->status.gotDisconnected);
	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!client1->status.gotConnected);
	BOOST_CHECK(!client1->status.gotDisconnected);
	BOOST_CHECK(!client1->status.gotReceived);
	BOOST_CHECK(!client2->status.gotReceived);
	BOOST_CHECK(!client2->status.gotTcpData);
	BOOST_CHECK(!client2->status.gotUdpData);

	BOOST_TEST_MESSAGE("ClientToServerDestinationNotNullCheck done");
}

BOOST_AUTO_TEST_CASE (ServerToClient1DataCheck)
{
	BFG::Base::Deco::h1("\nStarting ServerToClient1DataCheck");
	resetEventStatus();

	// Send to Client1
	BFG::Network::DataPayload payload1Tcp(testAppIdTcp, client_1_AppHandle, serverAppHandle, testMsg1.length(), testData1);
	BFG::Network::DataPayload payload1Udp(testAppIdUdp, client_1_AppHandle, serverAppHandle, testMsg1.length(), testData1);

	server->lane.emit(BFG::ID::NE_SEND, payload1Tcp);
	server->lane.emit(BFG::ID::NE_SEND_UDP, payload1Udp);

	EventStatus c1;
	c1.gotReceived = true;
	c1.gotTcpData  = true;
	c1.gotUdpData  = true;
	periodicWaitForEqual(c1, client1->status, boost::posix_time::milliseconds(5000));
	
	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!server->status.gotTcpData);
	BOOST_CHECK(!server->status.gotUdpData);
	BOOST_CHECK(client1->status.gotReceived);
	BOOST_CHECK(client1->status.gotTcpData);
	BOOST_CHECK(client1->status.gotUdpData);
	BOOST_CHECK(!client2->status.gotReceived);
	BOOST_CHECK(!client2->status.gotTcpData);
	BOOST_CHECK(!client2->status.gotUdpData);
	
	BOOST_TEST_MESSAGE("ServerToClient1DataCheck done");
}

BOOST_AUTO_TEST_CASE (ServerToClient2DataCheck)
{
	BFG::Base::Deco::h1("\nStarting ServerToClient1DataCheck");
	resetEventStatus();

	// Send to Client2
	BFG::Network::DataPayload payload2Tcp(testAppIdTcp, client_2_AppHandle, serverAppHandle, testMsg2.length(), testData2);
	BFG::Network::DataPayload payload2Udp(testAppIdUdp, client_2_AppHandle, serverAppHandle, testMsg2.length(), testData2);

	server->lane.emit(BFG::ID::NE_SEND, payload2Tcp);
	server->lane.emit(BFG::ID::NE_SEND_UDP, payload2Udp);

	EventStatus c2;
	c2.gotReceived = true;
	c2.gotTcpData  = true;
	c2.gotUdpData  = true;
	periodicWaitForEqual(c2, client2->status, boost::posix_time::milliseconds(5000));
	
	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!server->status.gotTcpData);
	BOOST_CHECK(!server->status.gotUdpData);
	BOOST_CHECK(!client1->status.gotReceived);
	BOOST_CHECK(!client1->status.gotTcpData);
	BOOST_CHECK(!client1->status.gotUdpData);
	BOOST_CHECK(client2->status.gotReceived);
	BOOST_CHECK(client2->status.gotTcpData);
	BOOST_CHECK(client2->status.gotUdpData);
	
	BOOST_TEST_MESSAGE("ServerToClient1DataCheck done");
}

BOOST_AUTO_TEST_CASE (Client1Disconnect)
{
	BFG::Base::Deco::h1("\nStarting Client1Disconnect");
	resetEventStatus();
	client1->lane.emit(BFG::ID::NE_DISCONNECT, BFG::Network::PeerIdT(0));

	EventStatus c1;
	c1.gotDisconnected = true;
	periodicWaitForEqual(c1, client1->status, boost::posix_time::milliseconds(5000));
	
	EventStatus s;
	s.gotDisconnected = true;
	periodicWaitForEqual(s, server->status, boost::posix_time::milliseconds(5000));
	
	BOOST_CHECK(client1->status.gotDisconnected);
	BOOST_CHECK(server->status.gotDisconnected);
	BOOST_CHECK(!client2->status.gotDisconnected);
	
	BOOST_TEST_MESSAGE("Client1Disconnect done");
}

BOOST_AUTO_TEST_CASE (ServerShutdown)
{
	BFG::Base::Deco::h1("\nStarting ServerShutdown");
	resetEventStatus();
	server->lane.emit(BFG::ID::NE_SHUTDOWN, BFG::Event::Void());

	// Must sleep since nothing will happen (best case).
	boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	
	// Should not receive any events since DISCONNECT was sent previously
	BOOST_CHECK(!server->status.gotConnected);
	BOOST_CHECK(!server->status.gotDisconnected);
	BOOST_CHECK(!server->status.gotReceived);
	BOOST_CHECK(!client1->status.gotConnected);
	BOOST_CHECK(!client1->status.gotDisconnected);
	BOOST_CHECK(!client1->status.gotReceived);

	BOOST_TEST_MESSAGE("ServerShutdown done");
}
