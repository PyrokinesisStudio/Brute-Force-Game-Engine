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

#include <Network/NetworkModule.h>

#include <boost/typeof/typeof.hpp>
#include <Network/Enums.hh>
#include <Network/Event.h>
#include <Network/Packet.h>

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

template <typename ProtocolT>
void NetworkModule<ProtocolT>::setFlushTimer(const long& waitTime_ms)
{
	if (waitTime_ms == 0)
		return;

	mTimer->expires_from_now(boost::posix_time::milliseconds(waitTime_ms));
	mTimer->async_wait(boost::bind(&NetworkModule<ProtocolT>::flushTimerHandler, this->shared_from_this(), _1));
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::setTcpDelay(bool on)
{
	boost::asio::ip::tcp::no_delay oldOption;
	mSocket->get_option(oldOption);
	boost::asio::ip::tcp::no_delay newOption(!on);
	mSocket->set_option(newOption);
	
	dbglog << "Set TCP_NODELAY from " << oldOption.value()
	       << " to " << newOption.value();
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::write(boost::asio::const_buffer packet, std::size_t size)
{
	dbglog << "NetworkModule::write: " << size << " Bytes";

	boost::asio::async_write
	(
		*mSocket,
		boost::asio::buffer(packet, size),
		boost::bind(&NetworkModule<ProtocolT>::writeHandler, this->shared_from_this(), _1, _2, packet)
	);
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::read()
{
	dbglog << "NetworkModule::read (" << mPeerId << ")";
	boost::asio::async_read
	(
		*mSocket, 
		boost::asio::buffer(mReadHeaderBuffer),
		boost::asio::transfer_exactly(HeaderSerializationT::size()),
		bind(&NetworkModule<ProtocolT>::readHeaderHandler, this->shared_from_this(), _1, _2)
	);
	dbglog << "NetworkModule::read Done";
}

// async Handler
template <typename ProtocolT>
void NetworkModule<ProtocolT>::flushTimerHandler(const error_code &ec)
{
	if (!ec)
	{
		flush();
		setFlushTimer(FLUSH_WAIT_TIME);
	}
	else if (ec.value() == boost::asio::error::operation_aborted)
	{
		warnlog << "NetworkModule: Flush timer was cancelled! (PeerID: " << mPeerId << ")";
	}
	else
	{
		printErrorCode(ec, "flushTimerHandler");
	}
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::readHeaderHandler(const error_code &ec, std::size_t bytesTransferred)
{
	dbglog << "NetworkModule::readHeaderHandler (" << bytesTransferred << ")";
	if (!ec) 
	{
		HeaderT header;
		header.deserialize(mReadHeaderBuffer);
		
		if (header.mDataLength == 0)
		{
			warnlog << "readHeaderHandler: Got empty Header! Disconnecting Client.";

			// Peer sends crap? Bye bye!
			mSocket.reset();
			return;
		}

		u16 headerChecksum = header.mHeaderChecksum;
		u16 ownHeaderChecksum = calculateHeaderChecksum(header);

		if (ownHeaderChecksum != headerChecksum)
		{
			warnlog << std::hex << std::uppercase 
			        << "readHeaderHandler: Got bad Header (Own CRC: "
			        << ownHeaderChecksum
			        << " Rcvd CRC: "
			        << headerChecksum
			        << "). Disconnecting Peer.";

			// Peer sends crap? Bye bye!
			mSocket.reset();
			return;
		}

		dbglog << "PacketSize: " << header.mDataLength;
		boost::asio::async_read
		(
			*mSocket, 
			boost::asio::buffer(mReadBuffer),
			boost::asio::transfer_exactly(header.mDataLength),
			bind(&NetworkModule<ProtocolT>::readDataHandler, this->shared_from_this(), _1, _2, header.mDataChecksum)
		);
	}
	else if (ec.value() == boost::asio::error::connection_reset)
	{
		dbglog << "NetworkModule: connection was closed!";
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readHeaderHandler");
	}
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::readDataHandler(const error_code &ec, std::size_t bytesTransferred, u32 packetChecksum)
{
	dbglog << "NetworkModule::readDataHandler (" << bytesTransferred << ")";
	if (!ec) 
	{
		u32 ownPacketChecksum = calculateChecksum(mReadBuffer.data(), bytesTransferred);

		if (ownPacketChecksum != packetChecksum)
		{
			warnlog << std::hex << std::uppercase 
			        << "readDataHandler: Got bad Packet (Own CRC: "
			        << ownPacketChecksum
			        << " Rcvd CRC: "
			        << packetChecksum
			        << "). Disconnecting Peer.";

			// Peer sends crap? Bye bye!
			mSocket.reset();
			return;
		}

		OPacket<ProtocolT> oPacket(boost::asio::buffer(mReadBuffer, bytesTransferred));
		onReceive(oPacket);
		read();
	}
	else if (ec.value() == boost::asio::error::connection_reset)
	{
		dbglog << "NetworkModule: connection was closed!";
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readDataHandler");
	}
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::writeHandler(const error_code &ec,
                                 std::size_t bytesTransferred,
                                 boost::asio::const_buffer buffer)
{
	dbglog << "NetworkModule::writeHandler: " << bytesTransferred << " Bytes written";
	if (ec)
	{
		printErrorCode(ec, "writeHandler");
	}
	mPool.free(const_cast<char*>(boost::asio::buffer_cast<const char*>(buffer)));
} 

template <typename ProtocolT>
void NetworkModule<ProtocolT>::dataPacketEventHandler(DataPacketEvent* e)
{
	switch(e->id())
	{
	case ID::NE_SEND:
		onSend(e->getData());
		break;
	default:
		warnlog << "NetworkModule: Can't handle event with ID: "
		        << e->id();
		break;
	}
}

template <typename ProtocolT>
void NetworkModule<ProtocolT>::printErrorCode(const error_code &ec, const std::string& method)
{
	warnlog << "This (" << this << ") " << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
}

} // namespace Network
} // namespace BFG
