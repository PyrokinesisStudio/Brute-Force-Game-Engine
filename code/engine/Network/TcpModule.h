/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

#ifndef BFG_NETWORK_TCPMODULE_H
#define BFG_NETWORK_TCPMODULE_H

#include <Network/NetworkModule.h>
#include <Network/Rtt.h>
#include <Network/Tcp.h>

namespace BFG {
namespace Network { 

class TcpModule : public NetworkModule<Tcp>
{
public:
	//! \brief Constructor
	//! \param[in] lane Event::lane the TcpModule is connected to 
	//! \param[in] service Asio service for the network connection
	//! \param[in] peerId ID of the module for identification over the network
	//! \param[in] localTime The local time of this module
	TcpModule(Event::Lane& lane,
	          boost::asio::io_service& service,
	          PeerIdT peerId,
	          boost::shared_ptr<Clock::StopWatch> localTime);
	
	virtual ~TcpModule();

	//! \brief Start synchronizing the local clock with the server
	void sendTimesyncRequest();

	//! \brief Enable or disable Nagle's Algorithm
	//! Enabling this could cause a delay in latency
	//! \param[in] on Enable delay or not
	void setTcpDelay(bool on)
	{
		boost::asio::ip::tcp::no_delay oldOption;
		socket().get_option(oldOption);
		boost::asio::ip::tcp::no_delay newOption(!on);
		socket().set_option(newOption);
		
		dbglog << "Set TCP_NODELAY from " << oldOption.value()
		       << " to " << newOption.value();
	}
	
	virtual void startReading()
	{
		setTcpDelay(false);
		NetworkModule<Tcp>::startReading();
	}

	virtual void startSending()
	{
		NetworkModule<Tcp>::startSending();
	}
		
	//! \brief Returns the socket of the connection
	//! \return socket of the connection
	Tcp::SocketT& socket()
	{
		return mSocket;
	}
	
private:
	// # Reading
	// ##########
	
	//! \brief Start asynchronous reading from the connected network module
	virtual void read();
	
	//! \brief Handler for the reading of the data header
	//! \param[in] ec Error code of boost asio
	//! \param[in] bytesTransferred size of the data received
	void readHeaderHandler(const boost::system::error_code &ec,
	                       std::size_t bytesTransferred);
	
	//! \brief Handler for the reading of the data
	//! \param[in] ec Error code of boost asio
	//! \param[in] bytesTransferred size of the data received
	//! \param[in] pacetChecksum Checksum of the data packet
	void readDataHandler(const boost::system::error_code &ec,
	                     std::size_t bytesTransferred,
	                     u32 packetChecksum);
	
	//! \brief Received data from the net is packed as a corresponding event 
	//! \param[in] data data array received from the network
	//! \param[in] size size of the data received
	void onReceive(OPacket<Tcp>& oPacket, PeerIdT peerId);

	// # Writing
	// ##########
	
	//! \brief Perform an asynchronous write of data to the connected network module
	//! \param[in] packet data to write over the net
	//! \param[in] size Size of the data set
	virtual void write(boost::asio::const_buffer packet, std::size_t size);
	
	//! \brief Send time criticle data to the connected network module
	//! Use this function to send packets as fast as possible.
	//! (e.g. for time synchronization). The payload will be sent almost
	//! immediately together with any other packets which were yet waiting
	//! for delivery. Therefore, delays caused by the event system or flush
	//! wait times are avoided.
	//! \param[in] payload Data to send
	void queueTimeCriticalPacket(DataPayload& payload);
	
	// # Time synchronization
	// #######################
	
	//! \brief Handles a request for time synchronization and sends the local timestamp back
	void onTimeSyncRequest();
	
	//! \brief Handles a response for time synchronization
	//! \param[in] serverTimestamp Timestamp of the server
	void onTimeSyncResponse(TimestampT serverTimestamp);

	//! \brief Calculates the offset between server and client time
	//! \param[in] serverTimestamp Timestamp of the server
	//! \param[out] offset Calculated offset
	//! \param[out] rtt Calculated round trip time
	void calculateServerTimestampOffset(u32 serverTimestamp, s32& offset, s32& rtt);
	
	//! \brief Sets the offset between the local time and server time
	//! \param[in] offset The time offset between client and server
	//! \param[in] rtt Round-Trip-Time of the data packet
	void setTimestampOffset(const s32 offset, const s32 rtt);
	
	Clock::StopWatch mRoundTripTimer;
	Rtt<s32, 10>     mRtt;
	s32              mTimestampOffset;
	
	// TODO: Use CreateBuffer
	HeaderSerializationT mReadHeaderBuffer;

	// TODO: Use CreateBuffer
	boost::array<char, Tcp::MAX_BYTE_RATE> mReadBuffer;
	
	Tcp::SocketT mSocket;
};

} // namespace Network
} // namespace BFG

#endif
