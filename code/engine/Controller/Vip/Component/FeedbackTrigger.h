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

//! \todo Find a better name for this module.
template <typename Parent>
class FeedbackTrigger : public Parent
{
public:
	explicit FeedbackTrigger(typename Parent::EnvT& env) :
		Parent(env),
		mFrequency(0),
		mTimeDeficit(0)
	{
		if (env.mFrequency > 0)
			mFrequency = ONE_SEC_IN_MICROSECS / env.mFrequency;
	}

	virtual void onFeedback(long microseconds_passed) = 0;

protected:
	long mFrequency;
	long mTimeDeficit;

private:
	//! Overwrites CommonBase::FeedTime()
	virtual void FeedTime(long microseconds_passed)
	{
		if (mFrequency == 0)
			return;
	
		mTimeDeficit += microseconds_passed;

		// Be very careful here.
		// Each tiny change may introduce a new race condition.
		while (Parent::NeedsTime() && mTimeDeficit >= mFrequency)
		{
			mTimeDeficit -= mFrequency;
			onFeedback(microseconds_passed);
		}
		
		if (mTimeDeficit > mFrequency)
		{
			mTimeDeficit = mFrequency;
		}
	}
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
