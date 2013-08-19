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

#include "ShipControl.h"

#include <Physics/Enums.hh>

#include "Globals.h"


ShipControl::ShipControl(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "ShipControl", pid)
{
	require("Physical");
	
	subLane()->connect(ID::GOE_CONTROL_YAW, this, &ShipControl::onGoeControlYaw, ownerHandle());
}

void ShipControl::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	Location go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

	bool setPos = false;

	// Simulate a wall
	if (std::abs(go.position.x) > DISTANCE_TO_WALL)
	{
		subLane()->emit(ID::PE_UPDATE_VELOCITY, v3::ZERO, ownerHandle());
		go.position.x = sign(go.position.x) * (DISTANCE_TO_WALL - 0.1f);
		setPos = true;
	}

	// Make sure it doesn't move too much on the z axis
	if (std::abs(go.position.z) - OBJECT_Z_POSITION > EPSILON_F)
	{
		go.position.z = OBJECT_Z_POSITION;
		setPos = true;
	}

	if (setPos)
		subLane()->emit(ID::PE_UPDATE_POSITION, go.position, ownerHandle());
}

void ShipControl::onGoeControlYaw(f32 factor)
{
	// Make the ship tilt a bit when moving
	qv4 tilt;
	qv4 turn;
	fromAngleAxis(turn, -90.0f * DEG2RAD, v3::UNIT_X);
	fromAngleAxis(tilt, factor * -45.0f * DEG2RAD, v3::UNIT_Z);

	qv4 newOrientation = turn * tilt;

	// Move it left or right
	v3 newVelocity = v3(factor * SHIP_SPEED_MULTIPLIER,0,0);

	subLane()->emit(ID::PE_UPDATE_ORIENTATION, newOrientation, ownerHandle());
	subLane()->emit(ID::PE_UPDATE_VELOCITY, newVelocity, ownerHandle());
}
