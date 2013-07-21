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
template <typename _IdT, typename _DestinationIdT, typename _SenderIdT>
struct Binder
{
	typedef _IdT IdT;
	typedef _DestinationIdT DestinationIdT;
	typedef _SenderIdT SenderIdT;
	
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

	~Binder()
	{
		BOOST_FOREACH(typename ConnectionMapT::value_type vt, mBindings)
		{
			Callable* c = boost::any_cast<Callable*>(vt.mBinding);
			delete c;
		}
	}
	
	template <typename PayloadT, typename FnT>
	void connect(const IdT id,
	             FnT fn, 
	             const DestinationIdT destination)
	{
		Callable* c = NULL;
		typename ConnectionMapT::iterator it = mBindings.find(boost::make_tuple(id, destination));
		if (it == mBindings.end())
		{
			c = new Binding<PayloadT, SenderIdT>(); 
			ConnectionT ea = {id, destination, c};
			mBindings.insert(ea);
		}
		else
		{
			c = boost::any_cast<Callable*>(it->mBinding);
		}
		
		Binding<PayloadT, SenderIdT>* b = static_cast<Binding<PayloadT, SenderIdT>*>(c);
		b->connect(fn);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(const IdT id,
	          const PayloadT& payload,
	          const DestinationIdT destination,
	          const SenderIdT sender) const
	{
		typename ConnectionMapT::const_iterator it = mBindings.find(boost::make_tuple(id, destination));
		if (it != mBindings.end())
		{
			Callable* c = boost::any_cast<Callable*>(it->mBinding);
			Binding<PayloadT, SenderIdT>* b = static_cast<Binding<PayloadT, SenderIdT>*>(c);
			try
			{
				b->emit(payload, sender);
				mCallSequence.push_back(c);
			}
			catch (IncompatibleTypeException& ex)
			{
				std::cout << ex.what() 
					<< " Trying to cast (" 
					<< ex.mEmittedType->name() 
					<< ") to (" 
					<< ex.mExpectedType->name() 
					<< "). "
					<< "Id/Dest: "
					<< id << "/" << destination
					<< std::endl;
				throw;
			}
		}
		//! \todo Else: event id not found in this EventBinder
	}

	// Verarbeitet alle events, die mit emit() gequeued wurden.
	void tick() const
	{
		std::for_each
		(
			mCallSequence.begin(),
			mCallSequence.end(),
			std::mem_fun(&Callable::call)
		);
		mCallSequence.clear();
	}
	
	ConnectionMapT mBindings;
	
	//! \todo Probably not thread-safe!
	mutable std::vector<Callable*> mCallSequence;
};

} // namespace Event
} // namespace BFG

#endif
