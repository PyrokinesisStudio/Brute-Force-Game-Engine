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

#ifndef BFG_CONTROLLER_VIP_BASE_H
#define BFG_CONTROLLER_VIP_BASE_H

#include <set>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <Event/Event.h>

#include <Controller/ControllerEvents_fwd.h>
#include <Controller/Enums.hh>
#include <Controller/Axis.h>
#include <Controller/Button.h>

// Used by Controller.cpp and VIPComponents.h
#define ONE_SEC_IN_MICROSECS 1000000

namespace BFG {
namespace Controller_ { 

//! \todo Define this in Core Time.h
typedef boost::units::quantity<boost::units::si::time, f32> TimeT;
typedef boost::units::quantity<boost::units::si::frequency, f32> FrequencyT;

class State;

namespace Vip { 
	
class CommonBase
{
public:
	CommonBase(Event::IdT aAid,
	           State* aState,
	           std::set<ButtonCodeT> aRelevantButtons,
	           std::set<AxisNumeratorT> aRelevantAxis,
	           ID::DeviceType aDevice);
	
	virtual ~CommonBase()
	{}
	
	//! \note There's no way around. A VIP must be able to receive
	//!       all types of data in order to work correctly.
	//!       I tried hard to put these two functions under a hat,
	//!       but I failed.
	virtual void FeedAxisData(ID::DeviceType,
	                          ID::AxisType,
	                          const AxisData<int>&)
	{
	}

	//! \see FeedAxisData()
	virtual void FeedButtonData(ID::DeviceType,
	                            ID::ButtonState,
	                            ButtonCodeT)
	{
	}

	Event::IdT getAction() const
	{
		return mAid;
	}
	
	//! Optional. Only used if feedback is needed.
	//! This gets always called by State.
	virtual void FeedTime(TimeT /*timeSinceLastTick*/)
	{
	}
	
	bool NeedsTime() const
	{
		return mUsingFeedback;
	}

	void EnableFeedback()
	{
		mUsingFeedback = true;
	}
	
	void DisableFeedback()
	{
		mUsingFeedback = false;		
	}

	template <typename T>
	void emit(T result)
	{
#ifdef CONTROLLER_DEBUG
		dbglog << "EMITTER WILL SEND: " << getAction()
			   << " WITH RESULT: " << getResult();
#endif

		mEventSubLane->emit(getAction(), result);
	}
	
public:
	std::set<ButtonCodeT>    mRelevantButtons;
	std::set<AxisNumeratorT> mRelevantAxis;
	ID::DeviceType           mDevice;

protected:
	Event::IdT mAid;
	State*     mState;

private:
	bool              mUsingFeedback;
	Event::SubLanePtr mEventSubLane;
};

} // namespace Vip

typedef boost::shared_ptr<Vip::CommonBase> VipPtrT;

namespace Vip {

template <typename DeviceT>
void assignToDevice(VipPtrT vip, DeviceT& device)
{
	if (! device.good())
		return;

	BOOST_FOREACH(AxisNumeratorT axis, vip->mRelevantAxis)
	{
		device.registerVIP(ID::AT_Normal, axis, vip);
	}
	
	BOOST_FOREACH(ButtonCodeT button, vip->mRelevantButtons)
	{
		device.registerVIP(button, vip);
	}
}

} // namespace Vip

} // namespace Controller_
} // namespace BFG

#endif
