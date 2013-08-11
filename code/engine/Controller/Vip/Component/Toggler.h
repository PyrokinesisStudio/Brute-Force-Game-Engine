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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_TOGGLER_H
#define BFG_CONTROLLER_VIP_COMPONENT_TOGGLER_H

namespace BFG {
namespace Controller_ { 
namespace Vip { 

template <typename Parent>
class Toggler : public Parent
{
public:
	explicit Toggler(typename Parent::EnvT& env) :
		Parent(env),
		mActivated(false),
		mHoldLogic(env.mHold)
	{		
	}
	
	virtual void FeedButtonData(ID::DeviceType /*dt*/,
	                            ID::ButtonState bs,
	                            ButtonCodeT /*code*/)
	{
		if (mHoldLogic)
			performHoldLogic();
		else
			performSwitchLogic(bs);
	}
	
	bool getResult() const
	{
		return mActivated;
	}
	
private:
	void performSwitchLogic(ID::ButtonState bs)
	{
		if (bs == ID::BS_Pressed)
		{
			mActivated = ! mActivated;
			Parent::emit(getResult());
		}
	}
	
	void performHoldLogic()
	{
		mActivated = ! mActivated;
		Parent::emit(getResult());
	}

	bool mActivated;   // false = off; true = on
	bool mHoldLogic;
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
