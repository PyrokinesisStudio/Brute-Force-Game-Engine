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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_VIRTUALAXISWITHSWING_H
#define BFG_CONTROLLER_VIP_COMPONENT_VIRTUALAXISWITHSWING_H

namespace BFG {
namespace Controller_ { 
namespace Vip { 

template <typename Parent>
class VirtualAxisWithSwing : public VirtualAxis<Parent>
{
public:
	using VirtualAxis<Parent>::mAxis;
	using VirtualAxis<Parent>::mLower;
	using VirtualAxis<Parent>::mRaise;
	using VirtualAxis<Parent>::mButtonCache;

	using VirtualAxis<Parent>::lower;
	using VirtualAxis<Parent>::raise;
	
	typedef AxisData<f32> AxisT;
	typedef typename AxisT::ValueT AxisValueT;

	explicit VirtualAxisWithSwing(typename Parent::EnvT& env) :
		VirtualAxis<Parent>(env),
		mSwing(env.mSwing),
		mAxisMode(env.mAxisMode)
	{
	}
	
	virtual void onFeedback(long /*microseconds_passed*/)
	{
		if (mButtonCache[mLower])
			lower();
		else
		if (mButtonCache[mRaise])
			raise();
		else
		{
			if (! swingBackAbsolute(mAxis, mSwing))
				Parent::DisableFeedback();
		}

		Parent::emit(getResult());
	}
	
	AxisValueT getResult() const
	{
		if (mAxisMode == ID::AM_Absolute)
			return mAxis.abs;

		else /* mAxisMode == ID::AM_Relative */
			return mAxis.rel;
	}

	f32 mSwing;
	ID::AxisMode mAxisMode;
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
