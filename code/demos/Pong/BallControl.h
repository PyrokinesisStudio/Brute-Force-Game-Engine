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

#ifndef __PONG_BALL_CONTROL_H_
#define __PONG_BALL_CONTROL_H_

#include <Core/Math.h>
#include <Model/GameObject.h>
#include <Model/Property/Concept.h>
#include <Physics/Enums.hh>

#include "Pong/PongDefinitions.h"

class BallControl : public BFG::Property::Concept
{
public:
	BallControl(BFG::GameObject& Owner, BFG::PluginId pid) :
	BFG::Property::Concept(Owner, "BallControl", pid)
	{
		require("Physical");
	}
	
	void internalUpdate(quantity<si::time, BFG::f32> timeSinceLastFrame)
	{
		using namespace BFG;

		if (getGoValue<bool>(ID::PV_Remote, ValueId::ENGINE_PLUGIN_ID))
			return;

		v3 position = getGoValue<v3>(ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);
		v3 newVelocity = getGoValue<v3>(ID::PV_Velocity, ValueId::ENGINE_PLUGIN_ID);

		bool updateVelocity = false;
		bool updatePosition = false;
		if (nearEnough(newVelocity, v3::ZERO, EPSILON_F))
			return;

		// Simulate a wall
		if (std::abs(position.x) > DISTANCE_TO_WALL)
		{
			newVelocity.x = std::abs(newVelocity.x) * -BFG::sign(position.x);
			updateVelocity = true;
		}

		// Make sure it doesn't move on the z axis
		if (std::abs(position.z - OBJECT_Z_POSITION) > BFG::EPSILON_F)
		{
			position.z = OBJECT_Z_POSITION;
			newVelocity.z = 0.0;
			updateVelocity = true;
			updatePosition = true;
		}
		
		// If it passed through a player bar, set it back to zero
		const f32 ball_reset_distance = 15.0f;
		
		bool lossDetected = false;

		// Upper Bar Loss?
		if (position.y > ball_reset_distance)
		{
			subLane()->emit(A_LOWER_BAR_WIN, Event::Void());
			lossDetected = true;
		}
		// Lower Bar Loss?
		else if (position.y < -ball_reset_distance)
		{
			subLane()->emit(A_UPPER_BAR_WIN, Event::Void());
			lossDetected = true;
		}

		if (lossDetected)
		{
			position = v3(0.0f, 0.0f, OBJECT_Z_POSITION);
			newVelocity   = BALL_START_VELOCITY;
			updatePosition = true;
			updateVelocity = true;
		}

		f32 currentSpeed = BFG::length(newVelocity);
		f32 startSpeed = BFG::length(BALL_START_VELOCITY);
		
		if (currentSpeed < startSpeed)
		{
			newVelocity *= startSpeed / currentSpeed;
			updateVelocity = true;
		}

		if (std::abs(newVelocity.y) < DESIRED_SPEED)
		{
			newVelocity.y *= std::abs(DESIRED_SPEED / newVelocity.y);
			updateVelocity = true;
		}

		if (updatePosition)
			subLane()->emit(ID::PE_UPDATE_POSITION, position, ownerHandle());

		if (updateVelocity)
			subLane()->emit(ID::PE_UPDATE_VELOCITY, newVelocity, ownerHandle());
	}
};

#endif //__PONG_BALL_CONTROL_H_
