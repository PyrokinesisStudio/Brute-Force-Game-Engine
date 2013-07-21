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

namespace BFG {
namespace Event { 

template <typename _LaneT>
struct BasicSubLane : boost::noncopyable
{
	typedef _LaneT LaneT;
	typedef typename LaneT::IdT            IdT;
	typedef typename LaneT::DestinationIdT DestinationIdT;
	typedef typename LaneT::SenderIdT      SenderIdT;
	typedef typename LaneT::BinderT        BinderT;
	
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
	
	void tick()
	{
		mBinder.tick();
	}

	void invalidateLane()
	{
		mValidLane = false;
	}

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

	LaneT&  mLane;
	BinderT mBinder;
	bool    mValidLane;
};

} // namespace Event
} // namespace BFG

#endif
