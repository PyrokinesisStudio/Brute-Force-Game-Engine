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

#ifndef BFG_EVENT_CONNECTABLE_H
#define BFG_EVENT_CONNECTABLE_H

#include <Event/MemberArity.h>
#include <Event/Void.h>

namespace BFG {
namespace Event { 

template <typename BinderT, typename IdT, typename DestinationIdT>
struct ConnectorV
{
	template <typename PayloadT, typename ObjectT, typename FnT>
	static void connect(BinderT& binder, const IdT& id,
	                    ObjectT* object,
	                    FnT fn,
	                    const DestinationIdT& destination = static_cast<DestinationIdT>(0))
	{
		binder.template connect<PayloadT>(id, boost::bind(fn, object), destination);
	}
};

template <typename BinderT, typename IdT, typename DestinationIdT>
struct ConnectorVS
{
	template <typename PayloadT, typename ObjectT, typename FnT>
	static void connect(BinderT& binder, const IdT& id,
	                    ObjectT* object,
	                    FnT fn,
	                    const DestinationIdT& destination = static_cast<DestinationIdT>(0))
	{
		binder.template connect<PayloadT>(id, boost::bind(fn, object, _2), destination);
	}
};

template <typename BinderT, typename IdT, typename DestinationIdT>
struct ConnectorP
{
	template <typename PayloadT, typename ObjectT, typename FnT>
	static void connect(BinderT& binder, const IdT& id,
	                    ObjectT* object,
	                    FnT fn,
	                    const DestinationIdT& destination = static_cast<DestinationIdT>(0))
	{
		binder.template connect<PayloadT>(id, boost::bind(fn, object, _1), destination);
	}
};

template <typename BinderT, typename IdT, typename DestinationIdT>
struct ConnectorPS
{
	template <typename PayloadT, typename ObjectT, typename FnT>
	static void connect(BinderT& binder, const IdT& id,
	                ObjectT* object,
	                FnT fn,
	                const DestinationIdT& destination = static_cast<DestinationIdT>(0))
	{
		binder.template connect<PayloadT>(id, boost::bind(fn, object, _1, _2), destination);
	}
};

template <typename BinderT>
struct Connectable
{
	USING_ENVELOPE(typename BinderT::EnvelopeT);
	
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
			ConnectorPS<BinderT, IdT, DestinationIdT>,
			ConnectorP<BinderT, IdT, DestinationIdT>
		>::type Connector;

		Connector::template connect<PayloadT>(mBinder, id, object, fn, destination);
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
			ConnectorVS<BinderT, IdT, DestinationIdT>,
			ConnectorV<BinderT, IdT, DestinationIdT>
		>::type Connector;

		Connector c;
		c.template connect<Void>(mBinder, id, object, fn, destination);
	}
	
	BinderT mBinder;
};

} // namespace Event
} // namespace BFG

#endif
