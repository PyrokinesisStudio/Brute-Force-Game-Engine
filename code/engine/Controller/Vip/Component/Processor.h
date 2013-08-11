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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_PROCESSOR_H
#define BFG_CONTROLLER_VIP_COMPONENT_PROCESSOR_H

namespace BFG {
namespace Controller_ { 
namespace Vip { 

void ButtonDataDebug(ID::DeviceType dt, ID::ButtonState bs, ButtonCodeT code)	
{
	std::string code_str = "[unknown device in use]";

	if (dt == ID::DT_Mouse)
	{
		code_str = ID::asStr(OIS::MouseButtonID(code));
	}
	else if (dt == ID::DT_Keyboard)
	{
		code_str = ID::asStr(ID::KeyboardButton(code));
	}
	else if (dt == ID::DT_Joystick)
	{
		code_str = ID::asStr(ID::JoystickButtonID(code));
	}
		
	dbglog << "BUTTONPROCESSOR GOT: " << ID::asStr(dt)
	       << " WITH STATE: " << ID::asStr(bs)
	       << " AND CODE: " << code_str;
}

void AxisDataDebug(ID::DeviceType dt, ID::AxisType at, const AxisData<int>& ad)
{
	dbglog << "AXISPROCESSOR GOT: " << ID::asStr(dt)
	       << " ON AXIS: " << ad.numerator
	       << " OF TYPE: " << ID::asStr(at)
	       << " WITH VALUE: " << ad.abs;
}

template
<
	typename AxisFeedback,
	typename ButtonFeedback,
	typename Environment,
	typename Parent = CommonBase
>
class Processor : public Parent
{
public:
	typedef Environment EnvT;

	explicit Processor(EnvT& env) :
		Parent(env.mAction,
		       env.mState,
		       env.mRelevantButtons,
		       env.mRelevantAxis,
		       env.mDevice)
	{
	}
	
	virtual void FeedAxisData(ID::DeviceType dt,
	                          ID::AxisType at,
	                          const AxisData<s32>& ad)
	{
#ifdef CONTROLLER_DEBUG
		AxisDataDebug(dt,at,ad);
#endif
		AxisFeedback::Setup(dt,at,ad,this);
	}

	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
#ifdef CONTROLLER_DEBUG
		ButtonDataDebug(dt,bs,code);
#endif
		ButtonFeedback::Setup(dt,bs,code,this);
	}
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
