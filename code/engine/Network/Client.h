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

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <Base/EntryPoint.h>
#include <Core/ClockUtils.h>

#include <Event/Event.h>

#include <Network/Defs.h>
#include <Network/Event_fwd.h>
#include <Network/Handshake.h>

class EventLoop;

namespace BFG {
namespace Network {

class TcpModule;
class UdpReadModule;
class UdpWriteModule;

using namespace boost::asio::ip;
using namespace boost::system;

//! This class represents a network client. It starts the connection to a server 
//! using a NetworkModule
class NETWORK_API Client
{
public:
	//! \brief Constructor
	//! \param[in] loop EventLoop of the EventSystem
	Client(Event::Lane& lane);
	~Client();

protected:
	Event::Lane& mLane;

private:
	//! \brief Stops all communication to and from the server
	void stop();

	//! \brief Tries to connect to a server
	//! \param[in] ip IP of the server
	//! \param[in] port Port of the server
	void startConnecting(const std::string& ip, const std::string& port);

	//! \brief Starts an asynchronous read of the server handshake
	void readHandshake();

	//! \brief Handler of an asio Resolver called after trying to resolve server ip and port
	//! \param[in] ec Error code of boost::asio
	//! \param[in] it Iterator of all the resolved connections
	void resolveHandler(const error_code &ec, tcp::resolver::iterator it);

	//! \brief Handler of an asio socket connect attempt
	//! \param[in] ec Error code of boost::asio
	void connectHandler(const error_code &ec);

	//! \brief Handler of the readHandshake attempt
	//! \param[in] ec Error code of boost::asio
	//! \param[in] bytesTransferred size of the data received
	void readHandshakeHandler(const error_code &ec, size_t bytesTransferred);

	//! \brief Starts to connect to the server
	//! \param[in] endpoint Data struct containing the server identification
	void onConnect(const EndpointT& endpoint);

	//! \brief Stops the communication
	//! \param[in] peerId ID of the NetworkModule to stop communicating
	void onDisconnect(const PeerIdT& peerId);
	
	//! \brief Sets the delay for each time synchronization request
	//! \param[in] waitTime_ms Time to wait in milliseconds
	void setTimeSyncTimer(const long& waitTime_ms);

	//! \brief Handler of the time synchronization timer
	//! \param[in] ec Error code of boost::asio
	void syncTimerHandler(const error_code &ec);

	// TODO: Move to Checksum.h and remove crc include
	//! \brief Calculates the checksum of a Handshake
	//! \param[in] hs The Handshake to calculate the checksum for
	//! \return Calculated checksum
	u16 calculateHandshakeChecksum(const Handshake& hs);

	boost::asio::io_service mService;
	boost::shared_ptr<tcp::resolver> mResolver;
	boost::thread mThread;
	
	// TODO: This works only for 7 weeks of server runtime!
	// TODO: This will wreak havoc after 7 weeks.
	// TODO: Reset this watch somehow.
	boost::shared_ptr<Clock::StopWatch> mLocalTime;

	Handshake::SerializationT mHandshakeBuffer;

	boost::shared_ptr<boost::asio::deadline_timer> mTimeSyncTimer;

	boost::shared_ptr<TcpModule> mTcpModule;

	boost::shared_ptr<UdpReadModule> mUdpReadModule;
	boost::shared_ptr<UdpWriteModule> mUdpWriteModule;

	PeerIdT mPeerId;
};

} // namespace Network
} // namespace BFG

#endif
