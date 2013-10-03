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

#ifndef BFG_EVENT_CATCHER_H
#define BFG_EVENT_CATCHER_H

#include <vector>

namespace BFG {
namespace Event { 

//! \brief Utility class for event arrival tests.
template <typename PayloadT>
struct Catcher
{
	typedef std::vector<PayloadT> PayloadsT;
	
	template <typename LaneT, typename IdT, typename DestinationIdT>
	Catcher(LaneT& lane, IdT eventId, DestinationIdT destinationId) :
	mSubLane(lane.createSubLane()),
	mEventCount(0)
	{
		mSubLane->connect(eventId, this, &Catcher<PayloadT>::callback, destinationId);
	}
	
	//! \brief Returns the number of event receivals.
	s32 count() const
	{
		return mEventCount;
	}

	//! \brief Returns all received payloads.
	const PayloadsT& payloads() const
	{
		return mPayloads;
	}

private:
	void callback(const PayloadT& payload)
	{
		++mEventCount;
		mPayloads.push_back(payload);
	}
	
	SubLanePtr mSubLane;
	s32        mEventCount;
	PayloadsT  mPayloads;
};

} // namespace Event
} // namespace BFG

#endif
