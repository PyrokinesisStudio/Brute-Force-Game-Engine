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

#include <Network/UdpModule.h>

#include <boost/asio/buffer.hpp>

#include <Base/Logger.h>
#include <Core/Utils.h>

#include <Network/CreateBuffer.h>
#include <Network/Defs.h>
#include <Network/Packet.h>
#include <Network/Udp.h>
#include <Network/UnreliableHeader.h>

namespace BFG {
namespace Network {

using namespace boost;

UdpModule::UdpModule(EventLoop* loop_,
                     asio::io_service& service,
                     shared_ptr<Clock::StopWatch> localTime,
                     const asio::ip::udp::endpoint& localEndpoint,
                     const asio::ip::udp::endpoint& serverEndpoint,
                     EndpointIdentificatorT endpointIdentificator) :
NetworkModule<Udp>(loop_, service, generateNetworkHandle(), localTime),
mSocket(service, localEndpoint),
mServerEndpoint(serverEndpoint),
mEndpointIdentificator(endpointIdentificator),
mLastRemoteSequenceNumber(0)
{
	dbglog << "Starting UdpModule(" << this << ")" << " with:";
	dbglog << "\tLocal Endpoint: " << localEndpoint;
	dbglog << "\tServer Endpoint: " << serverEndpoint;
	
	if (localEndpoint == serverEndpoint)
		dbglog << "\tThis is a Server UdpModule";
	else
		dbglog << "\tThis is a Client UdpModule";
}

UdpModule::~UdpModule()
{
	dbglog << "~UdpModule(" << this << ")";
	mSocket.close();
}

void UdpModule::useServerEndpointAsRemoteEndpoint()
{
	mRemoteEndpoint = mServerEndpoint;
}

void UdpModule::read()
{
	dbglog << "UDP: read()";
	socket().async_receive_from
	(
		boost::asio::buffer(mReadBuffer),
		mRemoteEndpoint,
		bind
		(
			&UdpModule::readHandler,
			this,
			_1,
			_2
		)
	);
	dbglog << "UDP: ~read()";
}

void UdpModule::readHandler(const system::error_code& ec, std::size_t bytesTransferred)
{
	dbglog << "UdpModule::readHandler() ec: " << ec;
	dbglog << "UdpModule::readHandler() got " << bytesTransferred << " bytes: " << std::string(mReadBuffer.data(), bytesTransferred);
	
	Udp::HeaderT header;
	Udp::HeaderT::SerializationT headerBuffer;
	std::copy(mReadBuffer.begin(), mReadBuffer.begin() + Udp::headerSize(), headerBuffer.data());

	header.deserialize(headerBuffer);
	
	dbglog << "\tSequence Number: " << header.mSequenceNumber;
	if (header.mSequenceNumber > mLastRemoteSequenceNumber)
	{
		mLastRemoteSequenceNumber = header.mSequenceNumber;
		
		// TODO: Use timestamp
		dbglog << "\tTimestamp: " << header.mTimestamp;

		// TODO: Results from this query can be cached and speeded up
		PeerIdT sender = mEndpointIdentificator(mRemoteEndpoint);
		dbglog << "\tSender " << mRemoteEndpoint << " was identified as " << sender;
		
		asio::const_buffer dataBuffer = asio::buffer(mReadBuffer, bytesTransferred) + Udp::headerSize();
		OPacket<Udp> oPacket(dataBuffer);
		onReceive(oPacket, sender);
	}
	else
	{
		warnlog << "Discarding old or duplicated UDP packet #" << header.mSequenceNumber;
	}
	
	read();
}

void UdpModule::onReceive(OPacket<Udp>& oPacket, PeerIdT peerId)
{
	dbglog << "UdpModule::onReceive";

	// TODO: Implement these
	s32 mTimestampOffset = 0;
	Rtt<s32, 10> mRtt;
	
	PayloadFactory payloadFactory(mTimestampOffset, mLocalTime, mRtt);
	
	while (oPacket.hasNextPayload())
	{
		DataPayload payload = oPacket.nextPayload(payloadFactory);
		
		try
		{
			dbglog << "UdpModule::onReceive: Emitting NE_RECEIVED to: " << payload.mAppDestination << " from: " << peerId;
			emit<DataPacketEvent>(ID::NE_RECEIVED, payload, payload.mAppDestination, peerId);
		}
		catch (std::exception& ex)
		{
			warnlog << ex.what();
		}
	}
}

void UdpModule::write(asio::const_buffer packet, std::size_t size)
{
	dbglog << "UdpModule::write() bytes: " << size;

	socket().async_send_to
	(
		asio::buffer(packet),
		mRemoteEndpoint,
		bind
		(
			&UdpModule::writeHandler,
			this,
			_1,
			_2,
			packet
		)
	);
}

} // namespace Network
} // namespace BFG
