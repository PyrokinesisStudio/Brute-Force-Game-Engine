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

#include <Network/TcpModule.h>

namespace BFG {
namespace Network { 

TcpModule::TcpModule(EventLoop* loop_,
                     boost::asio::io_service& service,
                     PeerIdT peerId,
                     boost::shared_ptr<Clock::StopWatch> localTime) :
NetworkModule<Tcp>(loop_, service, peerId, localTime),
mRoundTripTimer(Clock::milliSecond),
mTimestampOffset(0)
{}

TcpModule::~TcpModule()
{}

void TcpModule::sendTimesyncRequest()
{
	dbglog << "Sending timesync request";
	Network::DataPayload payload(ID::NE_TIMESYNC_REQUEST, 0, 0, 0, CharArray512T());
	queueTimeCriticalPacket(payload);
	mRoundTripTimer.start();
}

void TcpModule::read()
{
	
	dbglog << "TcpModule::read (" << mPeerId << ")";
	boost::asio::async_read
	(
		*socket(),
		boost::asio::buffer(mReadHeaderBuffer),
		boost::asio::transfer_exactly(HeaderSerializationT::size()),
		bind(&TcpModule::readHeaderHandler, boost::shared_static_cast<TcpModule>(shared_from_this()), _1, _2)
	);
	dbglog << "TcpModule::~read";
}

void TcpModule::readHeaderHandler(const error_code &ec, std::size_t bytesTransferred)
{
	dbglog << "TcpModule::readHeaderHandler (" << bytesTransferred << ")";
	if (!ec)
	{
		HeaderT header;
		header.deserialize(mReadHeaderBuffer);
		
		if (header.mDataLength == 0)
		{
			warnlog << "readHeaderHandler: Got empty Header! Disconnecting Client.";

			// Peer sends crap? Bye bye!
			socket().reset();
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
			socket().reset();
			return;
		}

		dbglog << "PacketSize: " << header.mDataLength;
		boost::asio::async_read
		(
			*socket(),
			boost::asio::buffer(mReadBuffer),
			boost::asio::transfer_exactly(header.mDataLength),
			bind(&TcpModule::readDataHandler, boost::shared_static_cast<TcpModule>(shared_from_this()), _1, _2, header.mDataChecksum)
		);
	}
	else if (ec.value() == boost::asio::error::connection_reset)
	{
		dbglog << "TcpModule: connection was closed!";
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readHeaderHandler");
	}
}

void TcpModule::readDataHandler(const error_code &ec, std::size_t bytesTransferred, u32 packetChecksum)
{
	dbglog << "TcpModule::readDataHandler (" << bytesTransferred << ")";
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
			socket().reset();
			return;
		}

		OPacket<Tcp> oPacket(boost::asio::buffer(mReadBuffer, bytesTransferred));
		onReceive(oPacket);
		read();
	}
	else if (ec.value() == boost::asio::error::connection_reset)
	{
		dbglog << "TcpModule: connection was closed!";
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readDataHandler");
	}
}

void TcpModule::onReceive(OPacket<Tcp>& oPacket)
{
	dbglog << "TcpModule::onReceive";

	PayloadFactory payloadFactory(mTimestampOffset, mLocalTime, mRtt);
	
	while (oPacket.hasNextPayload())
	{
		DataPayload payload = oPacket.nextPayload(payloadFactory);

		if (payload.mAppEventId == ID::NE_TIMESYNC_REQUEST)
		{
			onTimeSyncRequest();
			return;
		}
		else if (payload.mAppEventId == ID::NE_TIMESYNC_RESPONSE)
		{
			TimestampT serverTimestamp;
			memcpy(&serverTimestamp, payload.mAppData.data(), payload.mAppDataLen);
			onTimeSyncResponse(serverTimestamp);
		}
		
		try 
		{
			dbglog << "TcpModule::onReceive: Emitting NE_RECEIVED to: " << payload.mAppDestination << " from: " << mPeerId;
			emit<DataPacketEvent>(ID::NE_RECEIVED, payload, payload.mAppDestination, mPeerId);
		}
		catch (std::exception& ex)
		{
			warnlog << ex.what();
		}
	}
}

void TcpModule::queueTimeCriticalPacket(DataPayload& payload)
{
	dbglog << "TcpModule::queueTimeCriticalPacket -> onSend";
	onSend(payload);
	dbglog << "TcpModule::queueTimeCriticalPacket -> Flush";
	flush();
	dbglog << "TcpModule::queueTimeCriticalPacket -> Done";
}

void TcpModule::onTimeSyncRequest()
{
	dbglog << "Got time sync request from PeerId: " << mPeerId;

	TimestampT timestamp = mLocalTime->stop();

	CharArray512T ca512;
	memcpy(ca512.data(), &timestamp, sizeof(TimestampT));
	Network::DataPayload payload(ID::NE_TIMESYNC_RESPONSE, 0, 0, sizeof(TimestampT), ca512);

	queueTimeCriticalPacket(payload);
}

void TcpModule::onTimeSyncResponse(TimestampT serverTimestamp)
{
	dbglog << "Got time sync response from PeerId: " << mPeerId;

	s32 offset;
	s32 rtt;
	calculateServerTimestampOffset(serverTimestamp, offset, rtt);

	setTimestampOffset(offset, rtt);
}

void TcpModule::calculateServerTimestampOffset(u32 serverTimestamp, s32& offset, s32& rtt)
{
	// https://en.wikipedia.org/wiki/Cristian%27s_algorithm
	// offset = tS + dP/2 - tC
	u32 dP = mRoundTripTimer.stop();
	u32 tC = mLocalTime->stop();
	s32 serverOffset = serverTimestamp + dP / 2 - tC;

	dbglog << "Calculated server Timestamp Offset: " << serverOffset 
		<< " with RTT of " << dP;
	dbglog << "LocalTime was: " << tC;
	
	offset = serverOffset;
	rtt = dP;
}

void TcpModule::setTimestampOffset(const s32 offset, const s32 rtt)
{
	dbglog << "TcpModule:setTimestampOffset: "
		<< offset << "(" << offset - mTimestampOffset << ")" << ", "
		<< rtt << "(" << rtt - mRtt.last() << ")";

	mTimestampOffset = offset;
	mRtt.add(rtt);

	dbglog << "New avg rtt: " << mRtt.mean();
}

} // namespace Network
} // namespace BFG

