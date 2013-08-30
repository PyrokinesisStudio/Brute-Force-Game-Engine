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

#ifndef BFG_EVENT_SYNCHRONIZER_H
#define BFG_EVENT_SYNCHRONIZER_H

#include <vector>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <Base/NameCurrentThread.h>

namespace BFG {

namespace ID {
enum EventAction
{
	EA_FINISH = 1001
};
}

namespace Event { 

enum RunLevel
{
	RL1 = 1,
	RL2,
	RL3
};

template <typename _EnvelopeT>
struct BasicLane;

template <typename _LaneT>
struct BasicSynchronizer
{
	typedef _LaneT                                   LaneT;
	USING_ENVELOPE(typename LaneT::EnvelopeT);
	
	typedef typename std::multimap<RunLevel, LaneT*> LaneMapT;
	typedef typename LaneMapT::iterator              LaneMapIt;
	typedef std::pair<LaneMapIt, LaneMapIt>          RangeT;
	typedef boost::shared_ptr<boost::barrier>        BarrierPtrT;
	
	template<typename _EnvelopeT>
	friend struct BasicLane;

	BasicSynchronizer() :
	mFinish(4),
	mFinishEvent(false)
	{}
	
	~BasicSynchronizer()
	{
		finish();
	}
	
	void add(LaneT* lane)
	{
		mLanes.insert(std::make_pair(lane->mRunLevel, lane));
	}

	void start()
	{
		connectFinishEvent();

		startRunlevel(RL1);
		startRunlevel(RL2);
		startRunlevel(RL3);
	}

	void finish(bool finishOnEvent = false)
	{
		if (finishOnEvent)
		{
			while (!mFinishEvent)
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			}
		}

		// Nothing to finish if no threads exist within this Synchronizer.
		if (mThreads.empty())
			return;
		
		finishRunlevel(RL3);
		finishRunlevel(RL2);
		finishRunlevel(RL1);

		BOOST_FOREACH(boost::shared_ptr<boost::thread> t, mThreads)
		{
			t->join();
		}
		mThreads.clear();
		mBarriers.clear();
	}
	
private:
	void finishRunlevel(RunLevel runlevel)
	{
		u32 barrierCount = mLanes.count(runlevel) + 1;

		// Create barriers in order to wait for finishing threads.
		for (size_t i=0; i<10; ++i)
			mBarriers[runlevel].push_back(boost::make_shared<boost::barrier>(barrierCount));

		mFinish[runlevel] = true;

		for (size_t i=0; i<10; ++i)
			mBarriers[runlevel][i]->wait();

		dbglog << "Synchronizer finished runLevel " << runlevel;
	}

	void connectFinishEvent()
	{
		LaneMapIt it = mLanes.begin();

		if (it != mLanes.end())
		{
			it->second->connectV(ID::EA_FINISH, this, &BasicSynchronizer::onFinishEvent);
		}
	}

	void startRunlevel(RunLevel runlevel)
	{
		dbglog << "Synchronizer starting runlevel " << runlevel;

		// Create a new barrier to wait for all threads to finish
		// running their entry point(s).
		u32 runlevels = mLanes.count(runlevel);
		BarrierPtrT barrier(new boost::barrier(runlevels + 1));

		// Start a new thread for each lane
		RangeT ret = mLanes.equal_range(runlevel);
		for (LaneMapIt laneIt = ret.first; laneIt != ret.second; ++laneIt)
		{
			createThread(laneIt->second, barrier, runlevel);
		}

		mFinish[runlevel] = false;

		// Wait for all threads to finish (blocking)
		barrier->wait();
		dbglog << "Synchronizer initialized runLevel " << runlevel;
	}

	void createThread(LaneT* lane, BarrierPtrT runlevelBarrier, RunLevel runlevel)
	{
		mThreads.push_back
		(
			boost::make_shared<boost::thread>
			(
				boost::bind
				(
					&BasicSynchronizer<LaneT>::loop,
					this,
					lane,
					runlevelBarrier,
					runlevel
				)
			)
		);
	}

	template <typename PayloadT>
	void distributeToOthers(const IdT id, 
	                        const PayloadT& payload, 
	                        LaneT* lane, 
	                        const DestinationIdT destination,
	                        const SenderIdT sender)
	{
		BOOST_FOREACH(typename LaneMapT::value_type other, mLanes)
		{
			if (other.second != lane)
			{
				other.second->emitFromOther(id, payload, destination, sender);
			}
		}
	}
	
	void loop(LaneT* lane, BarrierPtrT entryPointBarrier, RunLevel runlevel)
	{
		// Run all entry points of this lane.
		lane->startEntries();

		// Then wait for the rest of this runlevel to finish.
		entryPointBarrier->wait();

		// Apply a name to this thread.
		if (!lane->mThreadName.empty())
			nameCurrentThread(lane->mThreadName);
		
		while (!mFinish[runlevel])
		{
			lane->tick();
		}
		
		// Perform a few last ticks in order to make sure that also the
		// latest events get delivered.
		for (std::size_t i=0; i<mBarriers[runlevel].size(); ++i)
		{
			mBarriers[runlevel][i]->wait();
			lane->tick();
		}

		// Trigger entry point de-init.
		lane->stopAndClearEntries();
		
		// *********************
		// * Thread exits here *
		// *********************
	}

	void onFinishEvent()
	{
		mFinishEvent = true;
	}

	LaneMapT                                       mLanes;
	std::vector<boost::shared_ptr<boost::thread> > mThreads;
	std::map<RunLevel, std::vector<BarrierPtrT> >  mBarriers;

	std::vector<bool> mFinish;
	bool mFinishEvent;
};

} // namespace Event
} // namespace BFG

#endif
