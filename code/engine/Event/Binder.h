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

#ifndef BFG_EVENT_BINDER_H
#define BFG_EVENT_BINDER_H

#include <boost/foreach.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <Event/Binding.h>
#include <Event/Callable.h>
#include <Event/Connection.h>

namespace BFG {
namespace Event { 

//! The Binder holds the map of EventIds with its corresponding Bindings
template <typename _IdT, typename _DestinationIdT>
struct Binder
{
	typedef _IdT IdT;
	typedef _DestinationIdT DestinationIdT;
	
	typedef Connection<IdT, DestinationIdT> ConnectionT;
	
	typedef boost::multi_index_container
	<
		ConnectionT,
		boost::multi_index::indexed_by
		<
			boost::multi_index::ordered_unique
			<
				boost::multi_index::composite_key
				<
					ConnectionT,
					boost::multi_index::member
					<
						ConnectionT,
						IdT,
						&ConnectionT::mEventId
					>,
					boost::multi_index::member
					<
						ConnectionT,
						DestinationIdT,
						&ConnectionT::mDestinationId
					>
				>
			>
		>
	> ConnectionMapT;

	
	template <typename PayloadT, typename FnT>
	void connect(int id, FnT fn, const int destination)
	{
		Callable* c = NULL;
		typename ConnectionMapT::iterator it = mSignals.find(boost::make_tuple(id, destination));
		if (it == mSignals.end())
		{
			c = new Binding<PayloadT>(); 
			ConnectionT ea = {id, destination, c};
			mSignals.insert(ea);
		}
		else
		{
			c = boost::any_cast<Callable*>(it->mBinding);
		}

		Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
		b->connect(fn);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(int id, const PayloadT& payload, const int destination)
	{
		typename ConnectionMapT::iterator it = mSignals.find(boost::make_tuple(id, destination));
		if (it != mSignals.end())
		{
			Callable* c = boost::any_cast<Callable*>(it->mBinding);
			Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
			b->emit(payload);
		}
		//! \todo Else: event id not found in this EventBinder
	}

	// Verarbeitet alle events, die mit emit() gequeued wurden.
	void tick()
	{
		BOOST_FOREACH(const ConnectionT& connection, mSignals)
		{
			Callable* c = boost::any_cast<Callable*>(connection.mBinding);
			c->call();
		}
	}
	
	ConnectionMapT mSignals;
};

} // namespace Event
} // namespace BFG

#endif
