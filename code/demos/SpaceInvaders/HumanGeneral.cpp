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

#include "HumanGeneral.h"

#include <Core/GameHandle.h>

#include <Model/Events/SectorEvent.h>
#include <Model/Data/GameObjectFactory.h>

#include "Globals.h"

HumanGeneral::HumanGeneral(EventLoop* loop,
						   boost::shared_ptr<BFG::Environment> environment) :
	Emitter(loop),
	mEnvironment(environment),
	mLastPowerupSpawned(0 * si::seconds)
{
}

void HumanGeneral::update(quantity<si::time, f32> timeSinceLastFrame)
{
	mLastPowerupSpawned += timeSinceLastFrame;

	if (mLastPowerupSpawned > 15.0f * si::seconds)
	{
		spawnPowerupAtRandomPosition();
		mLastPowerupSpawned = 0;
	}
}

void HumanGeneral::spawnPowerupAtRandomPosition() const
{
	ObjectParameter op;
	op = ObjectParameter();
	op.mHandle = generateHandle();
	op.mName = "Test Powerup";
	op.mType = "Powerup";
	op.mAngularVelocity = v3(0.0f, 5.0f, 0.0f);

	int spawnRange = static_cast<int>(DISTANCE_TO_WALL) * 2;
	int spawnPos = rand()%spawnRange - DISTANCE_TO_WALL;

	op.mLocation = v3
	(
		static_cast<f32>(spawnPos),
		-NEGATIVE_SHIP_Y_POSITION,
		OBJECT_Z_POSITION
	);
	emit<SectorEvent>(ID::S_CREATE_GO, op);
}
