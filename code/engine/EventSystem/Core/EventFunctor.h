/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

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

#ifndef __EVENT_FUNCTOR_H_
#define __EVENT_FUNCTOR_H_

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include <Core/Types.h>
#include <EventSystem/Core/EventDefs.h>

template < class EventListenerType, class EventType >
class SpecificEventFunctor 
{
public:
	typedef void (EventListenerType::*EventHandlerFunctionT)(EventType*);

	// constructor - takes pointer to an object and pointer to a member and stores
	SpecificEventFunctor(EventListenerType* pt2Object,
	                     EventHandlerFunctionT fpt,
	                     long long receiver /* 0 = global */) :
	mFpt(fpt),
	mPt2Object(pt2Object),
	mReceiver(receiver)
	{}

	virtual ~SpecificEventFunctor()
	{}

	//! Override function "Call" !
	virtual inline void call(EventType* event)
	{ 
		// Don't use the following line here, because that causes bad Branch Prediction
		//if ( !event->GetReceiver() || event->GetReceiver() == receiver )
		{
			// execute member function
			(*mPt2Object.*mFpt)(event);
		}
	};

	virtual EventListenerType* getEventListener() const
	{
		return mPt2Object;
	}

private:
	//! Pointer to EventHandlerFunction
	EventHandlerFunctionT mFpt;

	//! Pointer to EventHandlerObject
	EventListenerType* mPt2Object;

	long long mReceiver;
};



#endif //__EVENT_FUNCTOR_H_
