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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_VIRTUALAXIS_H
#define BFG_CONTROLLER_VIP_COMPONENT_VIRTUALAXIS_H

namespace BFG {
namespace Controller_ { 
namespace Vip { 

template <typename Parent>
class VirtualAxis : public Parent
{
public:
	typedef AxisData<f32> AxisT;
	typedef typename AxisT::ValueT AxisValueT;

	explicit VirtualAxis(typename Parent::EnvT& env) :
		Parent(env),
		mLower(env.mLower),
		mRaise(env.mRaise),
		mStep(env.mStep),
		mStopAtZero(env.mStopAtZero),
		mAxisMode(env.mAxisMode)
	{
	}
	
	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		mButtonCache[code] = (bs == ID::BS_Pressed);
		Parent::FeedButtonData(dt,bs,code);
	}
	
	virtual void onFeedback(TimeT /*timeSinceLastTick*/)
	{
		if (mButtonCache[mLower])
			lower();
		
		if (mButtonCache[mRaise])
			raise();

		Parent::emit(getResult());
	}

	AxisValueT getResult() const
	{
		if (mAxisMode == ID::AM_Absolute)
			return mAxis.abs;

		else /* mAxisMode == ID::AM_Relative */
			return mAxis.rel;
	}
	
	void raise()
	{
		//! \todo Make this configurable
		const float mUpperLimit = 1.0f;

		bool wasNegative = mAxis.abs < 0;
	
		mAxis.abs += mStep;
		mAxis.rel  = mStep;

		if (mStopAtZero && mAxis.abs >= 0 && wasNegative)
		{
			Parent::DisableFeedback();
			mAxis.reset();
		}

		if (mAxis.abs > mUpperLimit)
			mAxis.abs = mUpperLimit;
	}
	void lower()
	{
		//! \todo Make this configurable
		const float mLowerLimit = -1.0f;

		bool wasPositive = mAxis.abs > 0;

		mAxis.abs -= mStep;
		mAxis.rel  = -mStep;

		if (mStopAtZero && mAxis.abs <= 0 && wasPositive)
		{
			Parent::DisableFeedback();
			mAxis.reset();
		}

		if (mAxis.abs < mLowerLimit)
			mAxis.abs = mLowerLimit;
	}

protected:
	std::map<ButtonCodeT, bool> mButtonCache;
	
	ButtonCodeT mLower;
	ButtonCodeT mRaise;
	
	float mStep;
	bool  mStopAtZero;

public:
	AxisT mAxis;
	ID::AxisMode mAxisMode;
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
