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

#ifndef BFG_CONTROLLER_VIP_ENVIRONMENT_H
#define BFG_CONTROLLER_VIP_ENVIRONMENT_H

#include <string>
#include <set>

#include <Event/Event.h>

#include <Controller/Enums.hh>
#include <Controller/Axis.h>
#include <Controller/Button.h>

namespace BFG {
namespace Controller_ { 
	
class State;

namespace Vip { 

//! \brief
//! Environments contain configuration data for VIPs.
//! See VIPGenerator.h for how to create new ones. 
struct DefaultEnv
{
	BFG::Event::IdT mAction;
	State*   mState;
	
	//! This is a filter. Only buttons and axis which were put into these
	//! containers will be forwarded by the Delegater to the vip who gets
	//! the data of this struct.
	//! \see mRelevantAxis
	std::set<ButtonCodeT>    mRelevantButtons;
	
	//! \see mRelevantButtons
	std::set<AxisNumeratorT> mRelevantAxis;
	
	ID::DeviceType mDevice;
	ID::AxisMode   mAxisMode;
};

struct FeedbackEnv : public DefaultEnv
{
	long mFrequency;
};

struct SteerEnv : public FeedbackEnv
{
	float mSensitivity;
	float mRange;
	bool  mFlip;
	float mSwing;
};

struct ClickEnv : public DefaultEnv
{
	ID::ButtonState mFilter;
};

struct ModClickEnv : public ClickEnv
{
	ButtonCodeT mMod;
};

struct VAxisEnv : public FeedbackEnv
{
	ButtonCodeT mLower;
	ButtonCodeT mRaise;
	float       mStep;
	bool        mStopAtZero;
};

struct VAxisSwingEnv : public VAxisEnv
{
	float mSwing;
};

struct SequenceEnv : public ClickEnv
{
	std::string mSequence;
};

struct ToggleEnv : public DefaultEnv
{
	bool mHold;
};

} // namespace Vip
} // namespace Controller
} // namespace BFG

#endif
