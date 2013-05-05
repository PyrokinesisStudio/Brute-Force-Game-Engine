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

#include <Network/Server.h>

#include <boost/foreach.hpp>

#include <Core/GameHandle.h>

#include <Network/Event.h>
#include <Network/PrintErrorCode.h>
#include <Network/TcpModule.h>
#include <Network/UdpReadModule.h>
#include <Network/UdpWriteModule.h>

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

Server::Server(EventLoop* loop_) :
Emitter(loop_),
mLocalTime(new Clock::StopWatch(Clock::milliSecond))
{
	dbglog << "Server::Server()";

	mLocalTime->start();

	loop()->connect(ID::NE_LISTEN, this, &Server::controlEventHandler);
	loop()->connect(ID::NE_DISCONNECT, this, &Server::controlEventHandler);
	loop()->connect(ID::NE_SHUTDOWN, this, &Server::controlEventHandler);
}

Server::~Server()
{
	dbglog << "Server::~Server()";
	stop();
	loop()->disconnect(ID::NE_LISTEN, this);
	loop()->disconnect(ID::NE_DISCONNECT, this);
	loop()->disconnect(ID::NE_SHUTDOWN, this);

	loop()->disconnect(ID::NE_RECEIVED, this);
}

void Server::stop()
{
	dbglog << "Server::stop";

	mTcpModules.clear();
	mUdpWriteModules.clear();
	mUdpReadModule.reset();
	
	mService.stop();
	mThread.join();
}

void Server::startAccepting()
{
	dbglog << "Server::startAccepting";
	
	PeerIdT peerId = generateNetworkHandle();
	dbglog << "Server: Creating TcpModule #" << peerId << " for future client";
	boost::shared_ptr<TcpModule> tcpModule(new TcpModule(loop(), mService, peerId, mLocalTime));
	mTcpModules.insert(std::make_pair(peerId, tcpModule));

	dbglog << "Created Networkmodule(" << tcpModule << ")";
	mAcceptor->async_accept(tcpModule->socket(), bind(&Server::acceptHandler, this, _1, peerId));
}

void Server::sendHandshake(PeerIdT peerId)
{
	dbglog << "Server::sendHandshake peer ID: " << peerId;
	Handshake hs;
	hs.mPeerId = peerId;
	hs.mChecksum = calculateHandshakeChecksum(hs);
	hs.serialize(mHandshakeBuffer);
	
	boost::asio::async_write
	(
		mTcpModules[peerId]->socket(),
		boost::asio::buffer(mHandshakeBuffer.data(), Handshake::SerializationT::size()),
		boost::bind(&Server::writeHandshakeHandler, this, _1, _2, peerId)
	);
}

void Server::acceptHandler(const boost::system::error_code &ec, PeerIdT peerId)
{ 
	dbglog << "Server: Client connected: "
	       << mTcpModules[peerId]->socket().remote_endpoint().address()
	       << ":" 
	       << mTcpModules[peerId]->socket().remote_endpoint().port();

	if (ec)
	{
		printErrorCode(ec, "Server::acceptHandler", peerId);
		return;
	}

	sendHandshake(peerId);
	startAccepting();
}

void Server::writeHandshakeHandler(const error_code &ec, std::size_t bytesTransferred, PeerIdT peerId)
{
	if (ec)
	{
		printErrorCode(ec, "Server::writeHandshakeHandler", peerId);
		return;
	}
	
	dbglog << "Server: peerID was sent";
	mTcpModules[peerId]->startReading();
	mTcpModules[peerId]->startSending();

	emit<ControlEvent>(ID::NE_CONNECTED, peerId);
}

void Server::controlEventHandler(ControlEvent* e)
{
	switch(e->id())
	{
	case ID::NE_LISTEN:
		onListen(boost::get<u16>(e->getData()));
		break;
	case ID::NE_DISCONNECT:
		onDisconnect(boost::get<PeerIdT>(e->getData()));
		break;
	case ID::NE_SHUTDOWN:
		stop();
		break;
	default:
		warnlog << "Server: Can't handle event with ID: "
		        << e->id();
		break;
	}
}

void Server::onListen(const u16 port)
{
	if (!mAcceptor)
	{
		dbglog << "Server: Starting to listen on port " << port;

		// TCP
		boost::asio::ip::tcp::endpoint tcpServerEp = tcp::endpoint(tcp::v4(), port);
		mAcceptor.reset(new tcp::acceptor(mService, tcpServerEp));
		startAccepting();
		
		// UDP
		boost::shared_ptr<Udp::EndpointT> udpServerEp(new Udp::EndpointT(tcpServerEp.address(), tcpServerEp.port()));
		boost::shared_ptr<Udp::SocketT> udpServerSocket(new Udp::SocketT(mService, *udpServerEp));
		UdpReadModule::EndpointIdentificatorT identificator = boost::bind(&Server::identifyUdpEndpoint, this, _1);
		
		dbglog << "Server: Creating UdpModule as listener.";
		mUdpReadModule.reset(new UdpReadModule(
			loop(),
			mService,
			mLocalTime,
			udpServerSocket,
			identificator
		));
		mUdpReadModule->startReading();

		// Asio Loop
		mThread = boost::thread(boost::bind(&boost::asio::io_service::run, &mService));
	}
	else
	{
		warnlog << "Server is already listening!";
	}
}

void Server::onDisconnect(const PeerIdT& peerId)
{
	TcpModulesMap::iterator it = mTcpModules.find(peerId);
	if (it != mTcpModules.end())
	{
		mTcpModules[peerId].reset();
		mTcpModules.erase(it);
		emit<ControlEvent>(ID::NE_DISCONNECTED, peerId);
	}
}

PeerIdT Server::identifyUdpEndpoint(const boost::shared_ptr<Udp::EndpointT> remoteEndpoint)
{
	// Might exist yet
	UdpEndpointMap::const_iterator it = mUdpEndpoints.find(*remoteEndpoint);
	if (it != mUdpEndpoints.end())
		return it->second;
	
	const boost::asio::ip::address address = remoteEndpoint->address();
	BOOST_FOREACH(TcpModulesMap::value_type pair, mTcpModules)
	{
		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint tcpRemoteEndpoint = pair.second->socket().remote_endpoint(ec);
		if (ec)
			continue; // Probably not connected yet -> skip this module.

		dbglog << "Test " << tcpRemoteEndpoint.address() << " == " << address;
		
		bool ipEqual = (tcpRemoteEndpoint.address() == address);
		bool bound = (mUdpWriteModules.find(pair.first) != mUdpWriteModules.end());
		dbglog << "Bound? " << (bound?"Yes":"No");
		
		if (ipEqual && !bound)
		{
			dbglog << "Server: Creating UdpModule #" << pair.first << " as remote connection.";
			boost::shared_ptr<UdpWriteModule> udpModule(new UdpWriteModule(
				loop(),
				mService,
				pair.first,
				mLocalTime,
				mUdpReadModule->socket(),
				remoteEndpoint
			));
			udpModule->startSending();
			mUdpWriteModules[pair.first] = udpModule;
			mUdpEndpoints[*remoteEndpoint] = pair.first;
			return pair.first;
		}
	}

	dbglog << "Unable to identify remote endpoint: " << *remoteEndpoint;
	BOOST_FOREACH(TcpModulesMap::value_type pair, mTcpModules)
	{
		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint tcpRemoteEndpoint = pair.second->socket().remote_endpoint(ec);
		if (ec)
			continue; // Probably not connected yet -> skip this module.
		dbglog << "\tPeerID: " << pair.first << " TCP Address: " << tcpRemoteEndpoint;
	}
	dbglog << "Bound UDP modules:";
	BOOST_FOREACH(UdpWriteModulesMap::value_type pair, mUdpWriteModules)
	{
		dbglog << "\tPeerID: " << pair.first;
	}
	
	assert(! "TODO: What to do with unknown peer ids?");
	return 0xCDCDCDCD;
}

} // namespace Network
} // namespace BFG
