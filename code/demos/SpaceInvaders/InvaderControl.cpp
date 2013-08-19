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


#include "InvaderControl.h"
#include <Physics/Enums.hh>
#include "Globals.h"

InvaderControl::InvaderControl(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "InvaderControl", pid),
	mLastSidestep(0),
	mDirection(1.0f, -1.0f, 0),
	mDistanceFromStart(0,0,0)
{
	require("Physical");
}

void InvaderControl::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	Location go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

	v3 delta = INVADER_VELOCITY * timeSinceLastFrame.value() * mDirection;
	go.position += delta;

	mDistanceFromStart.x += delta.x;

	checkPosition(go.position, mDirection);

	go.orientation = INVADER_ORIENTATION;

	subLane()->emit(ID::PE_UPDATE_POSITION, go.position, ownerHandle());
	subLane()->emit(ID::PE_UPDATE_ORIENTATION, go.orientation, ownerHandle());
}

void InvaderControl::checkPosition(v3& position, v3& direction) const
{
	// Move into current direction, if farer than wall, move opposite
	if (std::abs(mDistanceFromStart.x) > INVADER_TURNING_DISTANCE)
	{
		const f32 xSpeed = 1.0f;
		direction.x = xSpeed * -sign(mDistanceFromStart.x);
	}
}
