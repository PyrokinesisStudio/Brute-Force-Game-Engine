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
	dbglog << "NetworkModule::queueTimeCriticalPacket -> onSend";
	onSend(payload);
	dbglog << "NetworkModule::queueTimeCriticalPacket -> Flush";
	flush();
	dbglog << "NetworkModule::queueTimeCriticalPacket -> Done";
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
	dbglog << "NetworkModule:setTimestampOffset: "
		<< offset << "(" << offset - mTimestampOffset << ")" << ", "
		<< rtt << "(" << rtt - mRtt.last() << ")";

	mTimestampOffset = offset;
	mRtt.add(rtt);

	dbglog << "New avg rtt: " << mRtt.mean();
}

} // namespace Network
} // namespace BFG

