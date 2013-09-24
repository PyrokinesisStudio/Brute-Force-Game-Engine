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

using namespace boost;

namespace BFG {
namespace Network { 

TcpModule::TcpModule(Event::Lane& lane,
                     asio::io_service& service,
                     PeerIdT peerId,
                     shared_ptr<Clock::StopWatch> localTime) :
NetworkModule<Tcp>(lane, service, peerId, localTime),
mRoundTripTimer(Clock::milliSecond),
mTimestampOffset(0),
mSocket(service)
{
	dbglog << "Creating TcpModule #" << peerId;
}

TcpModule::~TcpModule()
{
	dbglog << "Destroying TcpModule #" << mPeerId;
	mSocket.close();
}

void TcpModule::sendTimesyncRequest()
{
	dbglog << "Sending timesync request";
	DataPayload payload(ID::NE_TIMESYNC_REQUEST, 0, 0, 0, CharArray512T());
	queueTimeCriticalPacket(payload);
	mRoundTripTimer.start();
}

void TcpModule::read()
{
	dbglog << "TcpModule::read (" << mPeerId << ")";
	asio::async_read
	(
		socket(),
		asio::buffer(mReadHeaderBuffer),
		asio::transfer_exactly(HeaderSerializationT::size()),
		bind(&TcpModule::readHeaderHandler, shared_static_cast<TcpModule>(shared_from_this()), _1, _2)
	);
	dbglog << "TcpModule::~read";
}

void TcpModule::readHeaderHandler(const system::error_code &ec, std::size_t bytesTransferred)
{
	dbglog << "TcpModule::readHeaderHandler (" << bytesTransferred << ")";
	if (ec)
	{
		if (ec.value() == asio::error::connection_reset)
			dbglog << "TcpModule: connection was closed!";
		else
			printErrorCode(ec, "TcpModule::readHeaderHandler", mPeerId);
		mLane.emit(ID::NE_DISCONNECT, mPeerId);
		return;
	}

	HeaderT header;
	header.deserialize(mReadHeaderBuffer);
	
	if (header.mDataLength == 0)
	{
		warnlog << "readHeaderHandler: Got empty Header! Disconnecting Client.";

		// Peer sends crap? Bye bye!
		socket().close();
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
		socket().close();
		return;
	}

	dbglog << "PacketSize: " << header.mDataLength;
	asio::async_read
	(
		socket(),
		asio::buffer(mReadBuffer),
		asio::transfer_exactly(header.mDataLength),
		bind(&TcpModule::readDataHandler, shared_static_cast<TcpModule>(shared_from_this()), _1, _2, header.mDataChecksum)
	);
}

void TcpModule::readDataHandler(const system::error_code &ec, std::size_t bytesTransferred, u32 packetChecksum)
{
	dbglog << "TcpModule::readDataHandler (" << bytesTransferred << ")";
	if (ec)
	{
		if (ec.value() == asio::error::connection_reset)
			dbglog << "TcpModule: connection was closed!";
		else
			printErrorCode(ec, "TcpModule::readDataHandler", mPeerId);
		mLane.emit(ID::NE_DISCONNECT, mPeerId);
		return;
	}
	
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
		socket().close();
		return;
	}

	OPacket<Tcp> oPacket(asio::buffer(mReadBuffer, bytesTransferred));
	onReceive(oPacket, mPeerId);
	read();
}

void TcpModule::onReceive(OPacket<Tcp>& oPacket, PeerIdT peerId)
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
			return;
		}
		
		try 
		{
			dbglog << "TcpModule::onReceive: Emitting NE_RECEIVED to: " << payload.mAppDestination << " from: " << peerId;
			mLane.emit(ID::NE_RECEIVED, payload, payload.mAppDestination, peerId);
		}
		catch (std::exception& ex)
		{
			warnlog << ex.what();
		}
	}
}

void TcpModule::write(asio::const_buffer packet, std::size_t size)
{
	dbglog << "TcpModule::write: " << size << " Bytes";

	asio::async_write
	(
		socket(),
		asio::buffer(packet, size),
		bind
		(
			&TcpModule::writeHandler,
			shared_static_cast<TcpModule>(shared_from_this()),
			_1, _2,
			packet
		)
	);
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

