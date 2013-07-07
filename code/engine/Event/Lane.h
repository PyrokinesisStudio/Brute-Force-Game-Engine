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

#ifndef BFG_EVENT_LANE_H
#define BFG_EVENT_LANE_H

#include <boost/thread.hpp>

#include <Core/ClockUtils.h>
#include <Core/GameHandle.h>

#include <Event/Binder.h>
#include <Event/Binding.h>
#include <Event/Synchronizer.h>
#include <Event/TickData.h>

namespace BFG {
namespace Event { 

template <typename _IdT, typename _DestinationIdT>
struct BasicLane
{
	typedef _IdT IdT;
	typedef _DestinationIdT DestinationIdT;
	typedef BasicSynchronizer<BasicLane<IdT, DestinationIdT> > SynchronizerT;
	typedef Binder<IdT, DestinationIdT> BinderT;
	
	BasicLane(SynchronizerT& synchronizer, s32 ticksPerSecond) :
	mSynchronizer(synchronizer),
	mPlannedTimeInMs(1000/ticksPerSecond),
	sw(Clock::milliSecond)
	{
		sw.start();
		mSynchronizer.add(this);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(IdT id, const PayloadT& payload, const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template emit<PayloadT>(id, payload, destination);
		mSynchronizer.distributeToOthers(id, payload, this, destination);
	}
	
	template <typename PayloadT>
	void emitFromOther(IdT id, const PayloadT& payload, const DestinationIdT destination)
	{
		mBinder.template emit<PayloadT>(id, payload, destination);
	}

	template <typename FnT>
	void connectLoop(FnT loopFn)
	{
		mLoopBinding.connect(loopFn);
	}

	template <typename PayloadT, typename FnT>
	void connect(IdT id, FnT fn, DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, fn, destination);
	}

	void tick()
	{
		mLoopBinding.emit(TickData(sw.restart()));
		mLoopBinding.call();

		mBinder.tick();

		waitRemainingTime(sw.stop());
	}

	void waitRemainingTime(s32 consumedTime)
	{
		s32 remainingTime = mPlannedTimeInMs - consumedTime;
		if (remainingTime > 0)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
		}
	}
	
private:
	Clock::StopWatch sw;
	s32 mPlannedTimeInMs;
	SynchronizerT& mSynchronizer;
	BinderT mBinder;

	Binding<TickData> mLoopBinding;
};

} // namespace Event
} // namespace BFG

#endif
