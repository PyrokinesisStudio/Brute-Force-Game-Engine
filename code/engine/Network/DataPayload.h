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

#ifndef BFG_NETWORK_DATA_PAYLOAD_H
#define BFG_NETWORK_DATA_PAYLOAD_H

#include <Core/CharArray.h>
#include <Core/ClockUtils.h>
#include <Network/Rtt.h>
#include <Network/Segment.h>

namespace BFG {
namespace Network {

struct DataPayload
{
	DataPayload() :
	mAppEventId(0),
	mAppDestination(0),
	mAppSender(0),
	mAppDataLen(0),
	mTimestamp(0),
	mAge(0)
	{}
	
	DataPayload(u32 appEventId, GameHandle appDestination,
	            GameHandle appSender, size_t appDataLen,
	            CharArray512T appData, u32 timestamp = 0, u16 age = 0) :
	mAppEventId(appEventId),
	mAppDestination(appDestination),
	mAppSender(appSender),
	mAppDataLen(appDataLen),
	mAppData(appData),
	mTimestamp(timestamp),
	mAge(age)
	{}
	     
	//! Id of the actual event on application-level
	u32 mAppEventId;

	//! GameHandle of the actual receiver on application-level
	GameHandle mAppDestination;

	//! GameHandle of the actual sender on application-level
	GameHandle mAppSender;
	
	//! Actual length (not capacity) of the to be send/received data.
	size_t mAppDataLen;
	
	//! Actual data to be transmitted on application-level (serialized)
	CharArray512T mAppData;

	//! Timestamp is only used by NE_RECEIVED and represents the current server time
	u32 mTimestamp;
	
	//! How old the data actually is
	u16 mAge;
};

class PayloadFactory
{
public:
	PayloadFactory(s32 timestampOffset,
	               boost::shared_ptr<Clock::StopWatch> localtime,
	               Rtt<s32, 10> rtt) :
	mTimestampOffset(timestampOffset),
	mLocalTime(localtime),
	mRtt(rtt)
	{}
	
	DataPayload create(const Segment& segment, const CharArray512T& data) const
	{
		u32 currentServerTimestamp = mTimestampOffset + mLocalTime->stop();
		
		DataPayload payload
		(
			segment.appEventId,
			segment.destinationId,
			segment.senderId,
			segment.dataSize,
			data,
			currentServerTimestamp,
			mRtt.mean() / 2
		);
		return payload;
	};
	
	s32 mTimestampOffset;
	boost::shared_ptr<Clock::StopWatch> mLocalTime;
	Rtt<s32, 10> mRtt;
};

} // namespace Network
} // namespace BFG

#endif
