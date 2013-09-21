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

#ifndef __PONG_BAR_CONTROL_H_
#define __PONG_BAR_CONTROL_H_

#include <Core/Location.h>
#include <Core/Math.h>

#include <Model/Enums.hh>
#include <Model/GameObject.h>
#include <Model/Property/Concept.h>
#include <Physics/Enums.hh>

#include "Pong/PongDefinitions.h"

class BarControl : public BFG::Property::Concept
{
public:
	BarControl(BFG::GameObject& Owner, BFG::PluginId pid) :
	BFG::Property::Concept(Owner, "BarControl", pid),
	mInitialized(false)
	{
		require("Physical");
		subLane()->connect(BFG::ID::GOE_CONTROL_YAW, this, &BarControl::onYaw);
	}
	
	void onYaw(BFG::f32 yaw)
	{
		if (getGoValue<bool>(BFG::ID::PV_Remote, BFG::ValueId::ENGINE_PLUGIN_ID))
			return;

		subLane()->emit
		(
			BFG::ID::PE_UPDATE_VELOCITY,	
			v3(yaw * BAR_MAX_SPEED, 0, 0),
			ownerHandle()
		);
	}

	void internalUpdate(quantity<si::time, BFG::f32> timeSinceLastFrame)
	{
		using namespace BFG;

		if (getGoValue<bool>(ID::PV_Remote, ValueId::ENGINE_PLUGIN_ID))
			return;

		v3 position = getGoValue<v3>(BFG::ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);

		if (!mInitialized)
		{
			subLane()->emit(ID::PE_UPDATE_POSITION, position, ownerHandle());
			subLane()->emit(ID::PE_UPDATE_ORIENTATION, qv4::IDENTITY, ownerHandle());
			subLane()->emit(ID::PE_UPDATE_ROTATION_VELOCITY, v3::ZERO, ownerHandle());
			mInitialized = true;
		}

		// Simulate a wall
		if (std::abs(position.x) > DISTANCE_TO_WALL)
		{
			subLane()->emit(ID::PE_UPDATE_VELOCITY, v3::ZERO, ownerHandle());
			position.x = sign(position.x) * (DISTANCE_TO_WALL - 0.01f);
			subLane()->emit(ID::PE_UPDATE_POSITION, position, ownerHandle());
		}
	
		// Make sure it doesn't move to much on the z axis
		if (std::abs(position.z) - OBJECT_Z_POSITION > EPSILON_F)
		{
			position.z = OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET;
			subLane()->emit(ID::PE_UPDATE_POSITION, position, ownerHandle());
		}

	}
	
	bool mInitialized;
};

#endif //__PONG_BAR_CONTROL_H_
