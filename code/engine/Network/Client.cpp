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

#include <Network/Client.h>

#include <boost/crc.hpp>
#include <Base/Logger.h>
#include <Network/Event.h>
#include <Network/PrintErrorCode.h>
#include <Network/TcpModule.h>
#include <Network/UdpReadModule.h>
#include <Network/UdpWriteModule.h>

namespace BFG {
namespace Network{

Client::Client(EventLoop* loop_) :
Emitter(loop_),
mLocalTime(new Clock::StopWatch(Clock::milliSecond))
{
	dbglog << "Client::Client()";
	mLocalTime->start();

	mResolver.reset(new tcp::resolver(mService));
	
	mTimeSyncTimer.reset(new boost::asio::deadline_timer(mService));

	loop()->connect(ID::NE_CONNECT, this, &Client::controlEventHandler);
	loop()->connect(ID::NE_DISCONNECT, this, &Client::controlEventHandler);
	loop()->connect(ID::NE_SHUTDOWN, this, &Client::controlEventHandler);

	dbglog << "Client: Creating TcpModule";
	mTcpModule.reset(new TcpModule(loop(), mService, UNIQUE_PEER, mLocalTime));
}

Client::~Client()
{
	dbglog << "Client::~Client()";
	stop();

	loop()->disconnect(ID::NE_CONNECT, this);
	loop()->disconnect(ID::NE_DISCONNECT, this);
	loop()->disconnect(ID::NE_SHUTDOWN, this);
	loop()->disconnect(ID::NE_RECEIVED, this);

	if (mResolver)
		mResolver->cancel();

	mResolver.reset();
}

void Client::stop()
{
	dbglog << "Client::stop";
	
	mUdpReadModule.reset();
	mUdpWriteModule.reset();
	
	if (mTcpModule)
	{
		mTcpModule->socket().shutdown(boost::asio::socket_base::shutdown_both);
		mTcpModule->socket().close();
		mTcpModule.reset();
	}
	
	mService.stop();
	mThread.join();
}

void Client::startConnecting(const std::string& ip, const std::string& port)
{
	dbglog << "Client::startConnecting(" << ip << ":" << port << ")";
	boost::asio::ip::tcp::resolver::query query(ip, port);
	mResolver->async_resolve(query, boost::bind(&Client::resolveHandler, this, _1, _2));
}

void Client::readHandshake()
{
	dbglog << "Client::readHandshake";
	boost::asio::async_read
	(
		mTcpModule->socket(),
		boost::asio::buffer(mHandshakeBuffer),
		boost::asio::transfer_exactly(Handshake::SerializationT::size()),
		bind(&Client::readHandshakeHandler, this, _1, _2)
	);
}

void Client::resolveHandler(const error_code &ec, tcp::resolver::iterator it)
{ 
	dbglog << "TcpModule::resolveHandler";
	if (!ec)
	{
		mTcpModule->socket().async_connect(*it, bind(&Client::connectHandler, this, _1));
	}
	else
		printErrorCode(ec, "resolveHandler", UNIQUE_PEER);
}

void Client::connectHandler(const error_code &ec)
{
	dbglog << "Client::connectHandler";
	if (!ec) 
	{
		readHandshake();
	}
	else
	{
		printErrorCode(ec, "connectHandler", UNIQUE_PEER);
	}
}

//! \brief Helper function for UDP endpoint identification on client-side.
//! \return Always UNIQUE_PEER
static PeerIdT uniquePeer(const boost::shared_ptr<Udp::EndpointT>&)
{
	return UNIQUE_PEER;
}

void Client::readHandshakeHandler(const error_code &ec, size_t bytesTransferred)
{
	dbglog << "Client::readHandshakeHandler (" << bytesTransferred << ")";

	if (ec)
	{
		printErrorCode(ec, "Client::syncTimerHandler", UNIQUE_PEER);
		return;
	}

	Handshake hs;
	hs.deserialize(mHandshakeBuffer);
	
	u16 hsChecksum = hs.mChecksum;
	u16 ownHsChecksum = calculateHandshakeChecksum(hs);
	
	if (ownHsChecksum != hsChecksum)
	{
		warnlog << std::hex << std::uppercase 
			<< "Client: readHandshakeHandler: Got bad PeerId (Own CRC: "
			<< ownHsChecksum
			<< " Rcvd CRC: "
			<< hsChecksum
			<< "). Disconnecting Peer.";

		// Peer sends crap? Bye bye!
		// TODO: Notify Application
		mTcpModule->socket().close();
		return;
	}

	dbglog << "Client: Received peer ID: " << hs.mPeerId;
	mPeerId = hs.mPeerId;

	mTcpModule->startReading();
	mTcpModule->startSending();

	boost::asio::ip::tcp::endpoint tcpServerEp = mTcpModule->socket().remote_endpoint();
	boost::shared_ptr<boost::asio::ip::udp::endpoint> udpServerEp(new boost::asio::ip::udp::endpoint(tcpServerEp.address(), tcpServerEp.port()));
	boost::asio::ip::udp::endpoint udpLocalEp(udp::endpoint(udp::v4(), RANDOM_PORT));
	boost::shared_ptr<Udp::SocketT> socket(new Udp::SocketT(mService, udpLocalEp));
	
	dbglog << "Client: Creating UdpReadModule";
	mUdpReadModule.reset(new UdpReadModule(
		loop(),
		mService,
		mLocalTime,
		socket,
		uniquePeer
	));
	mUdpReadModule->startReading();

	dbglog << "Client: Creating UdpWriteModule";
	mUdpWriteModule.reset(new UdpWriteModule(
		loop(),
		mService,
		UNIQUE_PEER,
		mLocalTime,
		socket,
		udpServerEp
	));
	mUdpWriteModule->startSending();
	mUdpWriteModule->pingRemote();
	
	emit<ControlEvent>(ID::NE_CONNECTED, mPeerId);

	mTcpModule->sendTimesyncRequest();
	setTimeSyncTimer(TIME_SYNC_WAIT_TIME);
}

void Client::controlEventHandler(ControlEvent* e)
{
	switch(e->id())
	{
	case ID::NE_CONNECT:
		onConnect(boost::get<EndpointT>(e->getData()));
		break;
	case ID::NE_DISCONNECT:
	case ID::NE_SHUTDOWN:
		onDisconnect(UNIQUE_PEER);
		break;
	default:
		warnlog << "Client: Can't handle event with ID: "
			<< e->id();
		break;
	}

}

void Client::onConnect(const EndpointT& endpoint)
{
	startConnecting(endpoint.get<0>().data(), endpoint.get<1>().data());
	mThread = boost::thread(boost::bind(&boost::asio::io_service::run, &mService));
}

void Client::onDisconnect(const PeerIdT& peerId)
{
	stop();
	emit<ControlEvent>(ID::NE_DISCONNECTED, peerId);
}

void Client::setTimeSyncTimer(const long& waitTime_ms)
{
	if (waitTime_ms == 0)
		return;

	mTimeSyncTimer->expires_from_now(boost::posix_time::milliseconds(waitTime_ms));
	mTimeSyncTimer->async_wait(boost::bind(&Client::syncTimerHandler, this, _1));
}

void Client::syncTimerHandler(const error_code &ec)
{
	if (ec)
	{
		printErrorCode(ec, "Client::syncTimerHandler", UNIQUE_PEER);
		return;
	}

	mTcpModule->sendTimesyncRequest();
	setTimeSyncTimer(TIME_SYNC_WAIT_TIME);
}

u16 Client::calculateHandshakeChecksum(const Handshake& hs)
{
	boost::crc_16_type result;
	result.process_bytes(&(hs.mPeerId), sizeof(PeerIdT));
	return result.checksum();
}

} // namespace Network
} // namespace BFG
