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


#include <Physics/Enums.hh>
#include "Collectable.h"


Collectable::Collectable(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "Collectable", pid),
	mUsed(false)
{
	require("Physical");
	require("Destroyable");

	subLane()->connect(ID::PE_CONTACT, this, &Collectable::onCollect, ownerHandle());
}

void Collectable::onCollect(const Physics::ModulePenetration& mp)
{
	// Ignore additional PE_CONTACT events
	if (mUsed)
		return;

	mUsed = true;

	// Very Cannon specific stuff.
	s32 ammo = 5;
	subLane()->emit(ID::GOE_POWERUP, ammo, mp.mOtherModule);
}
