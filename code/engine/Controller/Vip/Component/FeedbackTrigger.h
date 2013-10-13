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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_FEEDBACKTRIGGER_H
#define BFG_CONTROLLER_VIP_COMPONENT_FEEDBACKTRIGGER_H

namespace BFG {
namespace Controller_ { 
namespace Vip { 

//! \brief Waits a 'tick' of time and triggers a callback.
//!
//! This function uses a user supplied frequency in order to simulate a
//! *fixed* tick rate behaviour.
//!
//! If a tick was missed due to not having enough time, the callback will be
//! fired multiple times in order to catch up.
//!
template <typename Parent>
class FeedbackTrigger : public Parent
{
public:
	explicit FeedbackTrigger(typename Parent::EnvT& env) :
	Parent(env),
	mFeedbackDuration(1.0f / env.mFrequency),
	mTimeDeficit(0.0f * boost::units::si::seconds)
	{}

	//! \brief Tick callback
	//! \note Only called if enough time has been passed and if
	//!       Parent::NeedsTime() returns true.
	virtual void onFeedback(TimeT timeSinceLastTick) = 0;

protected:
	//! The amount of time necessary to trigger feedback
	const TimeT mFeedbackDuration;
	
	//! Collects passed time
	TimeT mTimeDeficit;

private:
	//! Overwrites CommonBase::FeedTime()
	virtual void FeedTime(TimeT timeSinceLastTick)
	{
		if (mFeedbackDuration == 0 * boost::units::si::seconds)
			return;
	
		mTimeDeficit += timeSinceLastTick;

		// Be very careful here.
		// Each tiny change may introduce a new race condition.
		while (Parent::NeedsTime() && mTimeDeficit >= mFeedbackDuration)
		{
			mTimeDeficit -= mFeedbackDuration;
			onFeedback(timeSinceLastTick);
		}
		
		if (mTimeDeficit > mFeedbackDuration)
		{
			mTimeDeficit = mFeedbackDuration;
		}
	}
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
