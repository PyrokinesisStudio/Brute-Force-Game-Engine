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

#include <Network/UdpReadModule.h>

#include <Base/Logger.h>
#include <Network/Defs.h>

namespace BFG {
namespace Network { 

namespace asio = boost::asio;

UdpReadModule::UdpReadModule(Event::Lane& lane,
                             asio::io_service& service,
                             boost::shared_ptr<Clock::StopWatch> localTime,
                             boost::shared_ptr<Udp::SocketT> socket,
                             EndpointIdentificatorT endpointIdentificator) :
NetworkModule<Udp>(lane, service, UNIQUE_PEER, localTime),
mSocket(socket),
mEndpointIdentificator(endpointIdentificator)
{
	dbglog << "Creating UdpReadModule";

	assert(endpointIdentificator);
}

UdpReadModule::~UdpReadModule()
{
	mSocket->close();
	dbglog << "Destroying UdpReadModule";
}


void UdpReadModule::read()
{
	dbglog << "UdpReadModule::read()";

	boost::shared_ptr<Udp::EndpointT> remoteEndpoint(new Udp::EndpointT);
	
	socket()->async_receive_from
	(
		asio::buffer(mReadBuffer),
		*remoteEndpoint,
		bind
		(
			&UdpReadModule::readHandler,
			boost::static_pointer_cast<UdpReadModule>(shared_from_this()),
			_1,
			_2,
			remoteEndpoint
		)
	);
	dbglog << "UdpReadModule::read()";
}

void UdpReadModule::readHandler(const boost::system::error_code& ec,
                                std::size_t bytesTransferred,
                                boost::shared_ptr<Udp::EndpointT> remoteEndpoint)
{
	if (ec)
	{
		printErrorCode(ec, "UdpReadModule::readHandler", UNIQUE_PEER);
		mLane.emit(ID::NE_DISCONNECT, mPeerId);
		return;
	}
	
	dbglog << "UdpReadModule got " << bytesTransferred << " bytes: "
	       << std::string(mReadBuffer.data(), bytesTransferred);

	Udp::HeaderT header;
	Udp::HeaderT::SerializationT headerBuffer;
	std::copy(mReadBuffer.begin(), mReadBuffer.begin() + Udp::headerSize(), headerBuffer.data());
	header.deserialize(headerBuffer);
	
	PeerIdT sender = mEndpointIdentificator(remoteEndpoint);
	dbglog << "\tSender " << *remoteEndpoint << " was identified as " << sender;

	// Add a zero sequence number for new peer
	if (mLastSequenceNumbers.find(sender) == mLastSequenceNumbers.end())
		mLastSequenceNumbers.insert(std::make_pair(sender, 0));
	
	
	dbglog << "\tSequence Number: " << header.mSequenceNumber;
	if (header.mSequenceNumber > mLastSequenceNumbers[sender])
	{
		mLastSequenceNumbers[sender] = header.mSequenceNumber;
		
		// TODO: Use timestamp
		dbglog << "\tTimestamp: " << header.mTimestamp;

		asio::const_buffer dataBuffer = asio::buffer(mReadBuffer, bytesTransferred) + Udp::headerSize();
		OPacket<Udp> oPacket(dataBuffer);
		onReceive(oPacket, sender);
	}
	else
	{
		warnlog << "Discarding old or duplicated UDP packet:" << header.mSequenceNumber << " from " << sender;
	}
	
	read();
}

void UdpReadModule::onReceive(OPacket<Udp>& oPacket, PeerIdT peerId)
{
	dbglog << "UdpReadModule::onReceive";

	// TODO: Implement these
	s32 mTimestampOffset = 0;
	Rtt<s32, 10> mRtt;
	
	PayloadFactory payloadFactory(mTimestampOffset, mLocalTime, mRtt);
	
	while (oPacket.hasNextPayload())
	{
		DataPayload payload = oPacket.nextPayload(payloadFactory);
		
		if (payload.mAppEventId == ID::NE_PING_UDP)
		{
			dbglog << "UdpReadModule::onReceive: Got UDP Ping from #" << peerId;
			return;
		}
		
		try
		{
			dbglog << "UdpReadModule::onReceive: Emitting NE_RECEIVED to: " << payload.mAppDestination << " from: " << peerId;
			mLane.emit(ID::NE_RECEIVED, payload, payload.mAppDestination, peerId);
		}
		catch (std::exception& ex)
		{
			warnlog << ex.what();
		}
	}
}

} // namespace Network
} // namespace BFG
