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

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

#include <Core/ClockUtils.h>

#include <Event/Binder.h>
#include <Event/Binding.h>
#include <Event/EntryPoint.h>
#include <Event/Synchronizer.h>
#include <Event/TickData.h>

namespace BFG {
namespace Event {

template <typename _LaneT>
struct BasicSynchronizer;

template <typename _IdT, typename _DestinationIdT, typename _SenderIdT>
struct BasicLane : boost::noncopyable
{
	typedef _IdT IdT;
	typedef _DestinationIdT DestinationIdT;
	typedef _SenderIdT SenderIdT;
	typedef BasicSynchronizer<BasicLane<IdT, DestinationIdT, SenderIdT> > SynchronizerT;
	typedef Binder<IdT, DestinationIdT, SenderIdT> BinderT;
	typedef EntryPoint<BasicLane<IdT, DestinationIdT, SenderIdT> > EntryPointT;
	
	BasicLane(SynchronizerT& synchronizer, s32 ticksPerSecond) :
	mSynchronizer(synchronizer),
	mPlannedTimeInMs(1000/ticksPerSecond),
	mTickWatch(Clock::milliSecond),
	mEntriesStarted(false)
	{
		mTickWatch.start();
		mSynchronizer.add(this);
	}

	~BasicLane()
	{
		BOOST_FOREACH(EntryPointT& entryPoint, mEntryPoints)
		{
			entryPoint.stop();
		}
	}
	
	template <typename _LaneT>
	friend struct BasicSynchronizer;

	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(const IdT id, 
	          const PayloadT& payload, 
	          const DestinationIdT destination = static_cast<DestinationIdT>(0), 
	          const SenderIdT sender = static_cast<SenderIdT>(0))
	{
		try
		{
			mBinder.template emit<PayloadT>(id, payload, destination, sender);
		}
		catch (BFG::Event::IncompatibleTypeException&)
		{
			throw;
		}
		mSynchronizer.distributeToOthers(id, payload, this, destination, sender);
	}
	
	template <typename ObjectT>
	void connectLoop(ObjectT* object,
	                 void(ObjectT::*fn)(const TickData))
	{
		mLoopBinding.connect(boost::bind(fn, boost::ref(*object), _1));
	}

	//! Connect: const Payload, with Sender, non-const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
	             ObjectT* object,
	             void(ObjectT::*fn)(const PayloadT&, const SenderIdT&),
	             const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1, _2), destination);
	}

	//! Connect: non-const Payload, with Sender, non-const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
	             ObjectT* object,
	             void(ObjectT::*fn)(PayloadT, const SenderIdT&),
	             const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1, _2), destination);
	}

	//! Connect: const Payload, with Sender, const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
	             ObjectT* object,
	             void(ObjectT::*fn)(const PayloadT&, const SenderIdT&) const,
	             const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1, _2), destination);
	}

	//! Connect: non-const Payload, with Sender, const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
	             ObjectT* object,
	             void(ObjectT::*fn)(PayloadT, const SenderIdT&) const,
	             const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1, _2), destination);
	}
	
	//! Connect: const Payload, without Sender, non-const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
		ObjectT* object,
		void(ObjectT::*fn)(const PayloadT&),
		const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1), destination);
	}

	//! Connect: non-const Payload, without Sender, non-const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
		ObjectT* object,
		void(ObjectT::*fn)(PayloadT),
		const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1), destination);
	}

	//! Connect: const Payload, without Sender, const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
		ObjectT* object,
		void(ObjectT::*fn)(const PayloadT&) const,
		const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1), destination);
	}

	//! Connect: non-const Payload, without Sender, const handler
	template <typename PayloadT, typename ObjectT>
	void connect(const IdT id,
		ObjectT* object,
		void(ObjectT::*fn)(PayloadT) const,
		const DestinationIdT destination = static_cast<DestinationIdT>(0))
	{
		mBinder.template connect<PayloadT>(id, boost::bind(fn, boost::ref(*object), _1), destination);
	}

	template <typename EntryT>
	void addEntry()
	{
		mEntryPoints.push_back(new EntryT());
	}

	template <typename EntryT, typename ParameterT>
	void addEntry(ParameterT startParameter)
	{
		mEntryPoints.push_back(new EntryT(startParameter));
	}

private:

	void startEntries()
	{
		BOOST_FOREACH(EntryPointT& entryPoint, mEntryPoints)
		{
			entryPoint.run(this);
		}
		mEntriesStarted = true;
	}

	void stopEntries()
	{
		mEntriesStarted = false;
		BOOST_FOREACH(EntryPointT* entryPoint, mEntryPoints)
		{
			entryPoint->stop();
		}
	}

	template <typename PayloadT>
	void emitFromOther(const IdT id,
		const PayloadT& payload,
		const DestinationIdT destination,
		const SenderIdT sender)
	{
		mBinder.template emit<PayloadT>(id, payload, destination, sender);
	}

	void tick()
	{
		if (!mEntriesStarted)
			return;

		mLoopBinding.emit(TickData(mTickWatch.restart()), static_cast<SenderIdT>(0));
		mLoopBinding.call();

		mBinder.tick();
		waitRemainingTime(mTickWatch.stop());
	}

	void waitRemainingTime(const s32 consumedTime) const
	{
		s32 remainingTime = mPlannedTimeInMs - consumedTime;
		if (remainingTime > 0)
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
		}
	}
	
	SynchronizerT&    mSynchronizer;
	s32               mPlannedTimeInMs;
	Clock::StopWatch  mTickWatch;
	BinderT           mBinder;
	Binding<TickData, SenderIdT> mLoopBinding;
	boost::ptr_vector<EntryPointT> mEntryPoints;
	bool mEntriesStarted;
};

} // namespace Event
} // namespace BFG

#endif
