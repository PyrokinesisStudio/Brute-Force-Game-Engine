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

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>

#include <Core/GameHandle.h>

namespace BFG {
namespace Event { 

template <typename _LaneT>
struct BasicSynchronizer
{
	typedef _LaneT                         LaneT;
	typedef typename LaneT::IdT            IdT;
	typedef typename LaneT::DestinationIdT DestinationIdT;
	typedef typename LaneT::SenderIdT      SenderIdT;
	
	BasicSynchronizer() :
	mFinishing(false)
	{}
	
	~BasicSynchronizer()
	{
		finish();
	}
	
	void add(LaneT* lane)
	{
		mLanes.push_back(lane);
		mThreads.push_back(boost::make_shared<boost::thread>(boost::bind(&BasicSynchronizer<LaneT>::loop, this, lane)));
		boost::thread t;
	}

	void finish()
	{
		for (size_t i=0; i<10; ++i)
			mBarrier.push_back(boost::make_shared<boost::barrier>(mThreads.size()));

		mFinishing = true;
		
		BOOST_FOREACH(boost::shared_ptr<boost::thread> t, mThreads)
		{
			std::cout << "Joining thread " << t->get_id() << "\n";
			t->join();
		}
	}
	
	template <typename PayloadT>
	void distributeToOthers(const IdT id, 
	                        const PayloadT& payload, 
	                        LaneT* lane, 
	                        const DestinationIdT destination,
	                        const SenderIdT sender)
	{
		BOOST_FOREACH(LaneT* other, mLanes)
		{
			if (other != lane)
			{
				std::cout << ".";
				other->emitFromOther(id, payload, destination, sender);
			}
		}
	}
	
private:
	void loop(LaneT* lane)
	{
		while (!mFinishing)
		{
			lane->tick();
		}
		
		for (std::size_t i=0; i<mBarrier.size(); ++i)
		{
			std::cout << "Joining Barrier #" << i << " - " << boost::this_thread::get_id() << std::endl;
			mBarrier[i]->wait();
			std::cout << "TICK #" << i << " - " << boost::this_thread::get_id() << std::endl;
			lane->tick();
		}
		// -- Thread exits here --
	}

	std::vector<LaneT*> mLanes;
	std::vector<boost::shared_ptr<boost::thread> > mThreads;
	std::vector<boost::shared_ptr<boost::barrier> > mBarrier;

	bool mFinishing;
};

} // namespace Event
} // namespace BFG

#endif
