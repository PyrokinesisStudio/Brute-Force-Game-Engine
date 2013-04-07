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

#ifndef __SERVER_H__
#define __SERVER_H__

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include <Base/EntryPoint.h>
#include <Core/CharArray.h>
#include <Core/ClockUtils.h>
#include <Core/Types.h>
#include <EventSystem/Emitter.h>
#include <Network/Defs.h>
#include <Network/Event_fwd.h>
#include <Network/Handshake.h>

class EventLoop;

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

class TcpModule;
class UdpReadModule;
class UdpWriteModule;

//! This class represents a network server. It starts accepting connections from clients 
//! using several NetworkModules
class NETWORK_API Server : Emitter
{
public:
	//! \brief Constructor
	//! \param[in] loop EventLoop of the EventSystem
	Server(EventLoop* loop);
	~Server();

private:
	typedef std::map<PeerIdT, boost::shared_ptr<TcpModule> > TcpModulesMap;
	typedef std::map<PeerIdT, boost::shared_ptr<UdpWriteModule> > UdpWriteModulesMap;
	typedef std::map<boost::asio::ip::udp::endpoint, PeerIdT> UdpEndpointMap;
	
	//! \brief Stops all communication to and from all clients
	void stop();

	//! \brief Starts an asynchronous accept for clients
	void startAccepting();

	//! \brief Sends a Handshake to a connected client
	//! \param[in] peerId NetworkHandle of the client
	void sendHandshake(PeerIdT peerId);

	//! \brief Handler of an asio accept attempt
	//! \param[in] ec Error code of boost::asio
	//! \param[in] peerId NetworkHandle of the client
	void acceptHandler(const boost::system::error_code &ec, PeerIdT peerId);

	//! \brief Handler of a sendHandshake attempt
	//! \param[in] ec Error code of boost::asio
	//! \param[in] bytesTransferred size of the data written
	//! \param[in] peerId NetworkHandle of the client
	void writeHandshakeHandler(const error_code &ec, std::size_t bytesTransferred, PeerIdT peerId);

	//! \brief Handler to distribute ControlEvents
	//! \param[in] e Received ControlEvent
	void controlEventHandler(ControlEvent* ne);

	//! \brief Starts listening to a network port
	//! \param[in] port Network port the server is listening to
	void onListen(const u16 port);

	//! \brief Stops the communication
	//! \param[in] peerId ID of the NetworkModule to stop communicating
	void onDisconnect(const PeerIdT& peerId);
	
	PeerIdT identifyUdpEndpoint(const boost::shared_ptr<boost::asio::ip::udp::endpoint>);
	
	boost::asio::io_service mService;
	boost::shared_ptr<tcp::acceptor> mAcceptor;
	boost::thread mThread;

	// TODO: This works only for 7 weeks of server runtime!
	// TODO: This will wreak havoc after 7 weeks.
	// TODO: Reset this watch somehow.
	boost::shared_ptr<Clock::StopWatch> mLocalTime;

	Handshake::SerializationT mHandshakeBuffer;

	TcpModulesMap mTcpModules;
	UdpWriteModulesMap mUdpWriteModules;
	boost::shared_ptr<UdpReadModule> mUdpReadModule;
	
	UdpEndpointMap mUdpEndpoints;
};

} // namespace Network
} // namespace BFG

#endif
