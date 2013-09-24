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

#ifndef EVENT_STORAGE_H
#define EVENT_STORAGE_H

#include <Event/Event.h>

namespace BFG {

struct StorageCallable
{
	virtual ~StorageCallable()
	{}
	
	virtual void call(Event::SubLanePtr) = 0;
};

template <typename PayloadT>
struct EventBinding : public StorageCallable
{
	typedef boost::tuple<Event::IdT, PayloadT, Event::DestinationIdT, Event::SenderIdT> BufferT;

	EventBinding() :
	mTypeInfo(&typeid(PayloadT))
	{}
	
	void emit(const Event::IdT& id, const PayloadT& payload, const Event::DestinationIdT& destination, const Event::SenderIdT& sender)
	{
		if (typeid(PayloadT) != *mTypeInfo)
			throw Event::IncompatibleTypeException
			(
				"Binding::emit: Types incompatible.",
				&typeid(PayloadT),
				mTypeInfo
			);
		
		mPayloads.push(boost::make_tuple(id, payload, destination, sender));
	}
	
	virtual void call(Event::SubLanePtr subLane)
	{
		if (mPayloads.empty())
			return;

		while (!mPayloads.empty())
		{
			const BufferT& payload = mPayloads.front();

			subLane->emit(payload.template get<0>(), payload.template get<1>(), payload.template get<2>(), payload.template get<3>());
			mPayloads.pop();
		}
	}

private:

	std::queue<BufferT> mPayloads;
	const std::type_info* mTypeInfo;
	
};

struct EventStorage
{
	template <typename PayloadT>
	void emit(const Event::IdT id,
	          const PayloadT& payload,
			  const Event::DestinationIdT destination = NULL_HANDLE,
			  const Event::SenderIdT sender = NULL_HANDLE)
	{
		EventBinding<PayloadT>* b = new EventBinding<PayloadT>();
		try
		{
			b->emit(id, payload, destination, sender);
			mBindings.push_back(b);
		}
		catch (Event::IncompatibleTypeException& ex)
		{
			errlog << "EventStorage detected type mismatch on emit."
			       << " Id:" << id
			       << ", Destination:" << destination
			       << ", Emitted Type:" << ex.mEmittedType->name()
				   << ", Expected Type:" << ex.mExpectedType->name();
		}
	}

	void call(Event::SubLanePtr subLane)
	{
		BOOST_FOREACH(StorageCallable* c, mBindings)
		{
			c->call(subLane);
		}
		//! \todo memory leak in clearing mBindings
		mBindings.clear();
	}

	std::vector<StorageCallable*> mBindings;

};

} // namespace BFG

#endif