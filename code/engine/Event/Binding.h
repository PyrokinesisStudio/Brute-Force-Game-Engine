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

#ifndef BFG_EVENT_BINDING_H
#define BFG_EVENT_BINDING_H

#if defined (_WIN32)
	#define BOOST_PARAMETER_MAX_ARITY 7
#endif

#include <boost/foreach.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/mutex.hpp>

#include <Event/Callable.h>
#include <Event/IncompatibleTypeException.h>

namespace BFG {
namespace Event {

//! This is the binding of a collection of functions (boost::signals) with a collection of payloads
template <typename PayloadT, typename _SenderIdT>
struct Binding : public Callable
{
	typedef _SenderIdT SenderIdT;
	typedef boost::signals2::signal<void (const PayloadT&, const SenderIdT&)> SignalT;
	typedef boost::tuple<PayloadT, SenderIdT> BufferT;

	Binding() :
	mSignal(new SignalT),
	mTypeInfo(&typeid(PayloadT))
	{}
	
	template <typename FnT>
	void connect(FnT fn)
	{
		mSignal->connect(fn);
	}

	void emit(const PayloadT& payload, const SenderIdT& sender)
	{
		boost::mutex::scoped_lock sl(mFlipLocker);
		
		if (typeid(PayloadT) != *mTypeInfo)
			throw IncompatibleTypeException
			(
				"Binding::emit: Types incompatible.",
				&typeid(PayloadT),
				mTypeInfo
			);
		
		mBackPayloads.push_back(boost::make_tuple(payload, sender));
	}
	
	virtual void call()
	{
		flipPayloads();
		BOOST_FOREACH(const BufferT& buffer, mFrontPayloads)
		{
			signal()(buffer.template get<0>(), buffer.template get<1>());
		}
		mFrontPayloads.clear();
	}

private:

	void flipPayloads()
	{
		boost::mutex::scoped_lock sl(mFlipLocker);
		std::swap(mFrontPayloads, mBackPayloads);
	}

	const SignalT& signal() const
	{
		return *mSignal;
	}

	std::vector<BufferT> mFrontPayloads;
	std::vector<BufferT> mBackPayloads;

	boost::shared_ptr<SignalT> mSignal;
	const std::type_info* mTypeInfo;
	
	boost::mutex mFlipLocker;
};

} // namespace Event
} // namespace BFG

#endif
