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

#include <Core/Utils.h>

#include <Network/Event.h>
#include <Network/TcpModule.h>
#include <Network/UdpModule.h>

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

Server::Server(EventLoop* loop) :
mLoop(loop),
mLocalTime(new Clock::StopWatch(Clock::milliSecond))
{

	mLocalTime->start();

	mLoop->connect(ID::NE_LISTEN, this, &Server::controlEventHandler);
	mLoop->connect(ID::NE_DISCONNECT, this, &Server::controlEventHandler);
	mLoop->connect(ID::NE_SHUTDOWN, this, &Server::controlEventHandler);
}

Server::~Server()
{
	dbglog << "Server::~Server";
	stop();
	mLoop->disconnect(ID::NE_LISTEN, this);
	mLoop->disconnect(ID::NE_DISCONNECT, this);
	mLoop->disconnect(ID::NE_SHUTDOWN, this);

	mLoop->disconnect(ID::NE_RECEIVED, this);
}

void Server::stop()
{
	dbglog << "Server::stop";

	if (!mTcpModules.empty())
	{
		for(ModulesMap::iterator it = mTcpModules.begin(); it != mTcpModules.end(); ++it)
		{
			if (it->second && it->second->socket()->is_open())
				it->second->socket()->close();
		}

		mTcpModules.clear();
	}
	mService.stop();
	mThread.join();
}

void Server::startAccepting()
{
	dbglog << "Server::startAccepting";
	
	PeerIdT peerId = generateNetworkHandle();
	boost::shared_ptr<TcpModule> netModule(new TcpModule(mLoop, mService, peerId, mLocalTime));
	mTcpModules.insert(std::make_pair(peerId, netModule));

	dbglog << "Created Networkmodule(" << netModule << ")";
	mAcceptor->async_accept(*netModule->socket(), bind(&Server::acceptHandler, this, _1, peerId));
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
		*(mTcpModules[peerId]->socket()),
		boost::asio::buffer(mHandshakeBuffer.data(), Handshake::SerializationT::size()),
		boost::bind(&Server::writeHandshakeHandler, this, _1, _2, peerId)
	);
}

void Server::acceptHandler(const boost::system::error_code &ec, PeerIdT peerId)
{ 
	dbglog << "Client connected: "
	       << mTcpModules[peerId]->socket()->remote_endpoint().address()
	       << ":" 
	       << mTcpModules[peerId]->socket()->remote_endpoint().port();
	if (!ec) 
	{ 
		sendHandshake(peerId);
		startAccepting();
	}
	else
	{
		printErrorCode(ec, "acceptHandler");
	}
}

void Server::writeHandshakeHandler(const error_code &ec, std::size_t bytesTransferred, PeerIdT peerId)
{
	dbglog << "Server: peer ID was sent";
	mTcpModules[peerId]->startReading();

	Emitter e(mLoop);
	e.emit<ControlEvent>(ID::NE_CONNECTED, peerId);
}

void Server::controlEventHandler(ControlEvent* e)
{
	switch(e->getId())
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
		        << e->getId();
		break;
	}
}

void Server::onListen(const u16 port)
{
	if (!mAcceptor)
	{
		mAcceptor.reset(new tcp::acceptor(mService, tcp::endpoint(tcp::v4(), port)));
		dbglog << "Server started to listen to port " << port;
		startAccepting();
		mThread = boost::thread(boost::bind(&boost::asio::io_service::run, &mService));
		
		// UDP
		mUdpModule.reset(new UdpModule(mLoop, mService, port));
	}
	else
	{
		warnlog << "Server is allready listening!";
	}
}

void Server::onDisconnect(const PeerIdT& peerId)
{
	ModulesMap::iterator it = mTcpModules.find(peerId);
	if (it != mTcpModules.end())
	{
		mTcpModules[peerId].reset();
		mTcpModules.erase(it);
		Emitter e(mLoop);
		e.emit<ControlEvent>(ID::NE_DISCONNECTED, peerId);
	}
}

void Server::printErrorCode(const error_code &ec, const std::string& method)
{
	warnlog << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
}



} // namespace Network
} // namespace BFG
