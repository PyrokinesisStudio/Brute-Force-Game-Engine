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

#include <boost/uuid/uuid_io.hpp>

#include <Base/Logger.h>

#include <Network/Defs.h>
#include <Network/Exception.h>

namespace BFG {
namespace Network { 

namespace asio = boost::asio;

UdpReadModule::UdpReadModule(Event::Lane& lane,
                             boost::asio::io_service& service,
                             boost::shared_ptr<Clock::StopWatch> localTime,
                             boost::shared_ptr<Udp::SocketT> socket,
                             boost::shared_ptr<PeerIdentificator> peerIdentificator,
                             UdpWriteModuleCreatorT udpWriteModuleCreator
) :
NetworkModule<Udp>(lane, service, UNIQUE_PEER, localTime),
mSocket(socket),
mPeerIdentificator(peerIdentificator),
mUdpWriteModuleCreator(udpWriteModuleCreator)
{
	dbglog << "Creating UdpReadModule";
}

UdpReadModule::~UdpReadModule()
{
	mSocket->close();
	dbglog << "Destroying UdpReadModule";
}

void UdpReadModule::read()
{
	dbglog << "UdpReadModule::read() #" << mPeerId;

	auto remoteEndpoint = boost::make_shared<Udp::EndpointT>();
	dbglog << "UdpReadModule reading on " << *remoteEndpoint;

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
	dbglog << "UdpReadModule::~read() #" << mPeerId;
}

void UdpReadModule::readHandler(const boost::system::error_code& ec,
                                std::size_t bytesTransferred,
                                Udp::EndpointPtrT remoteEndpoint)
{
	if (ec)
	{
		printErrorCode(ec, "UdpReadModule::readHandler", UNIQUE_PEER);
		mLane.emit(ID::NE_DISCONNECT, mPeerId);
		read();
		return;
	}

	dbglog << "UdpReadModule got " << bytesTransferred << " bytes: ";
	       //<< std::string(mReadBuffer.data(), bytesTransferred);

	Udp::HeaderT header = parseHeader(mReadBuffer);

	// ---------------------------------
	// TODO: Implement these
	s32 mTimestampOffset = 0;
	Rtt<s32, 10> mRtt;
	PayloadFactory payloadFactory(mTimestampOffset, mLocalTime, mRtt);

	try
	{
		auto dataBuffer = boost::asio::buffer(mReadBuffer, bytesTransferred) + Udp::headerSize();
		OPacket<Udp> oPacket(dataBuffer);

		// Empty packet?
		if (!oPacket.hasNextPayload())
		{
			warnlog << "Received packet without data from " << *remoteEndpoint;
			read();
			return;
		}

		// Identify Peer
		PeerIdT senderId = (*mPeerIdentificator)(remoteEndpoint, oPacket);

		// Create UdpWriteModule and sequence number if it doesn't exist yet.
		if (mLastSequenceNumbers.find(senderId) == mLastSequenceNumbers.end())
		{
			dbglog << "UdpReadModule: Creating UdpWriteModule and sequence number";
			mUdpWriteModuleCreator(senderId, remoteEndpoint);
			mLastSequenceNumbers.insert(std::make_pair(senderId, 0));
		}

		dbglog << "\tGot UDP packet from #" << senderId;

		if (isOldPacket(header, senderId))
		{
			warnlog << "Discarding old or duplicated UDP packet:" << header.mSequenceNumber << " from " << senderId;
			read();
			return;
		}

		while (oPacket.hasNextPayload())
		{
			DataPayload payload = oPacket.nextPayload(payloadFactory);
			dbglog << "UdpReadModule::onReceive: Emitting NE_RECEIVED to: " << payload.mAppDestination << " from: " << senderId;
			mLane.emit(ID::NE_RECEIVED, payload, payload.mAppDestination, senderId);
		}
	}
	catch (UnknownPeerException& ex)
	{
		//! \todo Implement
		dbglog << "STUB: UnknownPeerException";
	}

	read();
}

bool UdpReadModule::isOldPacket(Udp::HeaderT header, PeerIdT senderId)
{
	if (header.mSequenceNumber > mLastSequenceNumbers[senderId])
	{
		mLastSequenceNumbers[senderId] = header.mSequenceNumber;
		//! \todo Use timestamp
		dbglog << "\tSequence Number: " << header.mSequenceNumber;
		dbglog << "\tTimestamp: " << header.mTimestamp;
		return false;
	}
	return true;
}

} // namespace Network
} // namespace BFG
