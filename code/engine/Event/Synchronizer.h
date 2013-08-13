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
namespace Event { 

enum RunLevel
{
	RL1,
	RL2,
	RL3
};

template <typename _IdT, typename _DestinationIdT, typename _SenderIdT>
struct BasicLane;

template <typename _LaneT>
struct BasicSynchronizer
{
	typedef _LaneT                         LaneT;
	typedef typename LaneT::IdT            IdT;
	typedef typename LaneT::DestinationIdT DestinationIdT;
	typedef typename LaneT::SenderIdT      SenderIdT;
	typedef typename std::multimap<RunLevel, LaneT*> LaneMapT;
	typedef typename LaneMapT::iterator LaneMapIt;
	typedef std::pair<LaneMapIt, LaneMapIt> RangeT;
	
	template<typename _IdT, typename _DestinationIdT, typename _SenderIdT>
	friend struct BasicLane;

	BasicSynchronizer() :
	mFinishing(false)
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
		u32 levelCount = mLanes.count(RL1);
		
		mStartBarrier.reset(new boost::barrier(levelCount + 1));

		RangeT ret = mLanes.equal_range(RL1);

		// Start a new thread for each lane
		for (LaneMapIt laneIt = ret.first; laneIt != ret.second; ++laneIt)
		{
			createThread(laneIt->second);
		}

		mStartBarrier->wait();

		std::cout << "RunLevel1 initialized";

		levelCount = mLanes.count(RL2);

		mStartBarrier.reset(new boost::barrier(levelCount + 1));

		ret = mLanes.equal_range(RL2);

		// Start a new thread for each lane
		for (LaneMapIt laneIt = ret.first; laneIt != ret.second; ++laneIt)
		{
			createThread(laneIt->second);
		}

		mStartBarrier->wait();

		std::cout << "RunLevel2 initialized";

		levelCount = mLanes.count(RL3);

		mStartBarrier.reset(new boost::barrier(levelCount + 1));

		ret = mLanes.equal_range(RL3);

		// Start a new thread for each lane
		for (LaneMapIt laneIt = ret.first; laneIt != ret.second; ++laneIt)
		{
			createThread(laneIt->second);
		}

		mStartBarrier->wait();

		std::cout << "RunLevel3 initialized";
	}

	void finish()
	{
		// Nothing to finish if no threads exist within this Synchronizer.
		if (mThreads.empty())
			return;
		
		// Create barriers in order to wait for finishing threads.
		for (size_t i=0; i<10; ++i)
			mBarrier.push_back(boost::make_shared<boost::barrier>(mThreads.size()));

		mFinishing = true;
		
		BOOST_FOREACH(boost::shared_ptr<boost::thread> t, mThreads)
		{
			t->join();
		}
	}
	
private:
	void createThread(LaneT* lane)
	{
		mThreads.push_back
		(
			boost::make_shared<boost::thread>
			(
				boost::bind
				(
					&BasicSynchronizer<LaneT>::loop,
					this,
					lane
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
		BOOST_FOREACH(LaneMapT::value_type other, mLanes)
		{
			if (other.second != lane)
			{
				other.second->emitFromOther(id, payload, destination, sender);
			}
		}
	}
	
	void loop(LaneT* lane)
	{
		lane->startEntries();

		mStartBarrier->wait();

		if (!lane->mThreadName.empty())
			nameCurrentThread(lane->mThreadName);
		
		while (!mFinishing)
		{
			lane->tick();
		}
		
		for (std::size_t i=0; i<mBarrier.size(); ++i)
		{
			mBarrier[i]->wait();
			lane->tick();
		}

		lane->stopAndClearEntries();
		// -- Thread exits here --
	}

	LaneMapT mLanes;
	std::vector<boost::shared_ptr<boost::thread> > mThreads;
	std::vector<boost::shared_ptr<boost::barrier> > mBarrier;
	boost::shared_ptr<boost::barrier> mStartBarrier;

	bool mFinishing;
};

} // namespace Event
} // namespace BFG

#endif
