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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_AXISCACHE_H
#define BFG_CONTROLLER_VIP_COMPONENT_AXISCACHE_H

#include <Core/Math.h>

namespace BFG {
namespace Controller_ { 
namespace Vip { 
	
//! Helper function for `swinging' the absolute part of an axis back to zero.
//! \param[out] axis The axis to be modified.
//! \param[in] swing Swing is applied stepwise. So this is equal to one step.
//! \return true If swingBack should still be called, false if otherwise.
template <typename AxisT>
bool swingBackAbsolute(AxisT& axis, typename AxisT::ValueT swing)
{
	if (std::abs(axis.abs) > EPSILON_F)
	{
		typename AxisT::ValueT sign = - (axis.abs / std::abs(axis.abs));

		// Don't swing too much (i.e.: from	0.1 to -0.1)
		if (swing > std::abs(axis.abs))
			swing = std::abs(axis.abs);

		axis.abs += swing * sign;
		axis.rel  = swing * sign;
		return true;
	}
	else
	{
		axis.reset();
		return false;
	}
}

//! Helper function for `swinging' the relative part of an axis back to zero.
//! \param[out] axis The axis to be modified.
//! \param[in] swing Swing is applied stepwise. So this is equal to one step.
//! \return true If swingBack should still be called, false if otherwise.
template <typename AxisT>
void swingBackRelative(AxisT& axis, typename AxisT::ValueT swing)
{
	typename AxisT::ValueT contrary_sign = - sign<f32>(axis.rel);

	if (std::abs(axis.rel) < swing)
		swing = std::abs(axis.rel);

	axis.rel += swing * contrary_sign;
}

template <typename Parent>
struct AxisCache : public Parent
{
	using Parent::DisableFeedback;

	typedef AxisData<f32> AxisT;
	typedef typename Parent::EnvT EnvT;

	explicit AxisCache(typename Parent::EnvT& env) :
		Parent(env),
		mSensitivity(env.mSensitivity),
		mFlip(env.mFlip),
		mRange(env.mRange),
		mSwing(env.mSwing)
	{
	}
	
	virtual void onFeedback(long /*microseconds_passed*/)
	{
		/**
			\note
			This swing logic is way better than in swingBack() and
			VirtualAxisWithSwing.
			We need the extra function swingBackRelative() here, because we want
			to decrease the relative part stepwise, not the absolute part. The
			reason for that is the use of relative pixel changes to simulate an
			absolute axis.
			In swingBackAbsolute(), the relative part is always set to the swing
			variable * contrary sign, therefore we wouldn't get any swing at
			all. 
		*/

		bool swinging = (std::abs(mSwing) > EPSILON_F);
		if (swinging)
			swingBackRelative(mAxis, mSwing);

		if (std::abs(mAxis.rel) < EPSILON_F)
		{
			mAxis.rel = 0;
			DisableFeedback();
		}

		this->Emit();

		if (!swinging)
			mAxis.rel = 0;		
	}

	virtual void FeedAxisData(ID::DeviceType dt,
	                          ID::AxisType at,
	                          const AxisData<s32>& ad)
	{
		mAxis = ad;

		if (mFlip)
		{
			mAxis.abs *= -1;
			mAxis.rel *= -1;
		}

		if (mRange > EPSILON_F)
		{
			mAxis.abs /= mRange;
			mAxis.rel /= mRange;
		}

		mAxis.rel *= mSensitivity;

		this->Emit();

		Parent::FeedAxisData(dt,at,ad);
	}

protected:
	AxisT mAxis;
	f32   mSensitivity;
	bool  mFlip;
	f32   mRange;
	f32   mSwing;
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
