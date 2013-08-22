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

#include <Model/Property/Concepts/SelfDestruction.h>

#include <Physics/Enums.hh>
#include <Model/Module.h>

namespace BFG {

SelfDestruction::SelfDestruction(GameObject& owner, PluginId pid) :
Property::Concept(owner, "SelfDestruction", pid)
{
	initvar(ID::PV_SelfDestructCountdown);

	require("Destroyable");	
}

void SelfDestruction::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	ModuleMapT::iterator it = mModules.begin();
	for (; it != mModules.end(); ++it)
	{
		GameHandle moduleHandle = it->first;
	
		if (mDone[moduleHandle])
			continue;
		
		mPassedTime[moduleHandle] += timeSinceLastFrame;
		
		f32& countdown = value<f32>(ID::PV_SelfDestructCountdown, moduleHandle);

		if (mPassedTime[moduleHandle] > countdown * si::seconds) 
		{
			mDone[moduleHandle] = true;
			
			mToDestroy.push(moduleHandle);
		}
	}
}

void SelfDestruction::internalSynchronize()
{
	while (!mToDestroy.empty())
	{
		GameHandle handle = mToDestroy.front();
		f32& damage = value<f32>(ID::PV_Damage, handle);
		
		damage = 999999.9f;

		mToDestroy.pop();
	}
}

} // namespace BFG
