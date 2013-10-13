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

#ifndef BFG_EVENT_SUBLANE_H
#define BFG_EVENT_SUBLANE_H

#include <boost/noncopyable.hpp>

#include <Event/Connectable.h>

namespace BFG {
namespace Event { 

template <typename _LaneT>
struct BasicSubLane : Connectable<typename _LaneT::BinderT>, boost::noncopyable
{
	typedef _LaneT                  LaneT;
	typedef typename LaneT::BinderT BinderT;
	
	USING_ENVELOPE(typename LaneT::EnvelopeT);
	
	using Connectable<BinderT>::mBinder;
	
	explicit BasicSubLane(LaneT& lane) :
	mLane(lane),
	mValidLane(true)
	{}
	
	template <typename PayloadT>
	void emit(const IdT id, 
	          const PayloadT& payload, 
	          const DestinationIdT destination = static_cast<DestinationIdT>(0), 
	          const SenderIdT sender = static_cast<SenderIdT>(0))
	{
		if (mValidLane)
			mLane.emit(id, payload, destination, sender);
		//! \todo else? Lane doesn't exist anymore. This call is invalid.
	}
	
	template <typename PayloadT>
	void subEmit(const IdT id, 
	          const PayloadT& payload, 
	          const DestinationIdT destination = static_cast<DestinationIdT>(0), 
	          const SenderIdT sender = static_cast<SenderIdT>(0))
	{
		mBinder.emit(id, payload, destination, sender);
	}
	
	void tick()
	{
		mBinder.tick();
	}

	void invalidateLane()
	{
		mValidLane = false;
	}

private:
	LaneT&  mLane;
	bool    mValidLane;
};

} // namespace Event
} // namespace BFG

#endif
