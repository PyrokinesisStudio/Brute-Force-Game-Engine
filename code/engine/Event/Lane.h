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
#include <Event/Void.h>

namespace BFG {
namespace Event {

template <typename _LaneT>
struct BasicSynchronizer;

template <typename _LaneT>
struct BasicSubLane;

template <typename _IdT, typename _DestinationIdT, typename _SenderIdT>
struct BasicLane : boost::noncopyable
{
	typedef _IdT IdT;
	typedef _DestinationIdT DestinationIdT;
	typedef _SenderIdT SenderIdT;
	typedef BasicLane<IdT, DestinationIdT, SenderIdT> This;
	typedef BasicSynchronizer<This> SynchronizerT;
	typedef BasicSubLane<This> SubLaneT;
	typedef Binder<IdT, DestinationIdT, SenderIdT> BinderT;
	typedef EntryPoint<BasicLane<IdT, DestinationIdT, SenderIdT> > EntryPointT;
	
	template <typename _LaneT>
	friend struct BasicSynchronizer;

	template <typename _LaneT>
	friend struct BasicSubLane;

	BasicLane(SynchronizerT& synchronizer, s32 ticksPerSecond, const std::string& threadName = std::string(), RunLevel runLevel = RL1):
	mThreadName(threadName),
	mSynchronizer(synchronizer),
	mPlannedTimeInMs(1000/ticksPerSecond),
	mTickWatch(Clock::milliSecond),
	mEntriesStarted(false),
	mRunLevel(runLevel)
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

		boost::mutex::scoped_lock sl(mFlipLocker);
		typename SubLaneContainerT::iterator it = mSubLanes.begin();
		for (; it != mSubLanes.end();)
		{
			boost::shared_ptr<SubLaneT> sublane = it->lock();
			if (sublane)
			{
				sublane->invalidateLane();
				++it;
			}
			else
			{
				it = mSubLanes.erase(it);
			}
		}
	}
	
	boost::shared_ptr<SubLaneT> createSubLane()
	{
		boost::shared_ptr<SubLaneT> sublane(new SubLaneT(*this));

		boost::mutex::scoped_lock sl(mFlipLocker);
		mSubLanes.push_back(sublane);
		return sublane;
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(const IdT id, 
	          const PayloadT& payload, 
	          const DestinationIdT destination = static_cast<DestinationIdT>(0), 
	          const SenderIdT sender = static_cast<SenderIdT>(0))
	{
		mBinder.template emit<PayloadT>(id, payload, destination, sender);
		mSynchronizer.distributeToOthers(id, payload, this, destination, sender);
		subEmit(id, payload, destination, sender);
	}
	
	template <typename ObjectT>
	void connectLoop(ObjectT* object,
	                 void(ObjectT::*fn)(TickData))
	{
		mLoopBinding.connect(boost::bind(fn, boost::ref(*object), _1));
	}

	//! Connect: handler with Payload
	template <typename ObjectT, typename FnT>
	void connect(const IdT& id,
	             ObjectT* object,
	             FnT fn,
	             const DestinationIdT& destination = static_cast<DestinationIdT>(0))
	{
		typedef typename member_arity<FnT>::arg1_type crPayloadT;
		typedef typename boost::remove_reference<crPayloadT>::type cPayloadT;
		typedef typename boost::remove_const<cPayloadT>::type PayloadT;
		typedef typename boost::mpl::if_c<
			member_arity<FnT>::arity == 2,
			ConnectorPS,
			ConnectorP
		>::type Connector;

		Connector c;
		c.template connect<PayloadT>(mBinder, id, object, fn, destination);
	}

	//! Connect: handler without Payload
	template <typename ObjectT, typename FnT>
	void connectV(const IdT& id,
	              ObjectT* object,
	              FnT fn,
	              const DestinationIdT& destination = static_cast<DestinationIdT>(0))
	{
		typedef typename boost::mpl::if_c<
			member_arity<FnT>::arity == 1,
			ConnectorVS,
			ConnectorV
		>::type Connector;

		Connector c;
		c.template connect<Void>(mBinder, id, object, fn, destination);
	}

	/** \brief Adds an entry point.
		
		This function adds a so-called "entry point" to this Lane. In
		order to facilitate library initialization, the event system may
		safely call multiple initializer functions. This is done by
		calling Synchronizer::startEntries().
		
		\see struct EntryPoint
		\see Synchronizer::startEntries()
	*/
	template <typename EntryT>
	void addEntry()
	{
		mEntryPoints.push_back(new EntryT());
	}
	
	/** \brief Adds an entry point, with a start parameter.
	
		\param[in] startParameter
			A parameter which is passed to the main class which
			serves as entry point.
			
		\tparam EntryT
			A class type which is used as entry point. Must inherit
			from class EntryPoint.
			
		\see Lane::addEntry()
		\note Use boost::ref for references.
	*/
	template <typename EntryT, typename ParameterT>
	void addEntry(ParameterT startParameter)
	{
		mEntryPoints.push_back(new EntryT(startParameter));
	}

	const std::string mThreadName;
	
private:
	template <typename Class> struct member_arity {};

	template <typename Class, typename R>
	struct member_arity<R (Class::*) (void)>
	{
		typedef R result_type;
		static const int arity = 0;
		static const bool c = false;
		typedef void arg1_type;
		typedef void arg2_type;
	};

	template <typename Class, typename R>
	struct member_arity<R (Class::*) (void) const>
	{
		typedef R result_type;
		static const int arity = 0;
		static const bool c = true;
		typedef void arg1_type;
		typedef void arg2_type;
	};

	template <typename Class, typename R, typename T1>
	struct member_arity<R (Class::*) (T1)>
	{
		typedef R result_type;
		static const int arity = 1;
		static const bool c = false;
		typedef T1 arg1_type;
		typedef void arg2_type;
	};

	template <typename Class, typename R, typename T1>
	struct member_arity<R (Class::*) (T1) const>
	{
		typedef R result_type;
		static const int arity = 1;
		static const bool c = true;
		typedef T1 arg1_type;
		typedef void arg2_type;
	};

	template <typename Class, typename R, typename T1, typename T2>
	struct member_arity<R (Class::*) (T1, T2)>
	{
		typedef R result_type;
		static const int arity = 2;
		static const bool c = false;
		typedef T1 arg1_type;
		typedef T2 arg2_type;
	};

	template <typename Class, typename R, typename T1, typename T2>
	struct member_arity<R (Class::*) (T1, T2) const>
	{
		typedef R result_type;
		static const int arity = 2;
		static const bool c = true;
		typedef T1 arg1_type;
		typedef T2 arg2_type;
	};

	struct ConnectorV
	{
		template <typename PayloadT, typename ObjectT, typename FnT>
		void connect(BinderT& binder, const IdT& id,
		             ObjectT* object,
		             FnT fn,
		             const DestinationIdT& destination = static_cast<DestinationIdT>(0))
		{
			binder.template connect<PayloadT>(id, boost::bind(fn, object), destination);
		}
	};

	struct ConnectorVS
	{
		template <typename PayloadT, typename ObjectT, typename FnT>
		void connect(BinderT& binder, const IdT& id,
		             ObjectT* object,
		             FnT fn,
		             const DestinationIdT& destination = static_cast<DestinationIdT>(0))
		{
			binder.template connect<PayloadT>(id, boost::bind(fn, object, _2), destination);
		}
	};

	struct ConnectorP
	{
		template <typename PayloadT, typename ObjectT, typename FnT>
		void connect(BinderT& binder, const IdT& id,
		             ObjectT* object,
		             FnT fn,
		             const DestinationIdT& destination = static_cast<DestinationIdT>(0))
		{
			binder.template connect<PayloadT>(id, boost::bind(fn, object, _1), destination);
		}
	};

	struct ConnectorPS
	{
		template <typename PayloadT, typename ObjectT, typename FnT>
		void connect(BinderT& binder, const IdT& id,
		             ObjectT* object,
		             FnT fn,
		             const DestinationIdT& destination = static_cast<DestinationIdT>(0))
		{
			binder.template connect<PayloadT>(id, boost::bind(fn, object, _1, _2), destination);
		}
	};

	void startEntries()
	{
		BOOST_FOREACH(EntryPointT& entryPoint, mEntryPoints)
		{
			entryPoint.run(this);
		}
		mEntriesStarted = true;
	}

	void stopAndClearEntries()
	{
		mEntriesStarted = false;
		BOOST_FOREACH(EntryPointT& entryPoint, mEntryPoints)
		{
			entryPoint.stop();
		}

		while (!mEntryPoints.empty())
			mEntryPoints.pop_back();
	}

	template <typename PayloadT>
	void emitFromOther(const IdT id,
		const PayloadT& payload,
		const DestinationIdT destination,
		const SenderIdT sender)
	{
		mBinder.template emit<PayloadT>(id, payload, destination, sender);
		subEmit(id, payload, destination, sender);
	}
	
	template <typename PayloadT>
	void subEmit(const IdT id,
		const PayloadT& payload,
		const DestinationIdT destination,
		const SenderIdT sender)
	{
		boost::mutex::scoped_lock sl(mFlipLocker);

		typename SubLaneContainerT::iterator it = mSubLanes.begin();
		for (; it != mSubLanes.end();)
		{
			boost::shared_ptr<SubLaneT> sublane = it->lock();
			if (sublane)
			{
				sublane->subEmit(id, payload, destination, sender);
				++it;
			}
			else
			{
				it = mSubLanes.erase(it);
			}
		}

	}

	void tick()
	{
		if (!mEntriesStarted)
			return;

		mLoopBinding.emit(TickData(mTickWatch.restart()), static_cast<SenderIdT>(0));
		mLoopBinding.call();

		mBinder.tick();
	
		//! copy of the original vector because called tick methods can emit and can create
		//! new sublanes.
		boost::mutex::scoped_lock sl(mFlipLocker);
		std::copy(mSubLanes.begin(), mSubLanes.end(), std::back_inserter(mSubLanesBackup));
		sl.unlock();

		typename SubLaneContainerT::iterator it = mSubLanesBackup.begin();
		for (; it != mSubLanesBackup.end();)
		{
			boost::shared_ptr<SubLaneT> sublane = it->lock();
			if (sublane)
			{
				sublane->tick();
				++it;
			}
			else
			{
				it = mSubLanesBackup.erase(it);
			}
		}
		
		mSubLanesBackup.clear();
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

	typedef std::vector<boost::weak_ptr<SubLaneT> > SubLaneContainerT;
	boost::mutex mFlipLocker;
	SubLaneContainerT mSubLanesBackup;
	SubLaneContainerT mSubLanes;

	RunLevel mRunLevel;
};

} // namespace Event
} // namespace BFG

#endif
