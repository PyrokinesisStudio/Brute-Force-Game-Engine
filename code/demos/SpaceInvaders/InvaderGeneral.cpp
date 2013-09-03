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


#include "InvaderGeneral.h"

#include <Model/Data/GameObjectFactory.h>

#include "Globals.h"

InvaderGeneral::InvaderGeneral(Event::SubLanePtr lane,
                               boost::shared_ptr<BFG::Environment> environment) :
mEnvironment(environment),
mLastShot(0),
mWaveCount(0),
mLane(lane)
{
	spawnWave();
	++mWaveCount;
	lane->connect(ID::S_DESTROY_GO, this, &InvaderGeneral::onDestroy);
}

void InvaderGeneral::onDestroy(GameHandle handle)
{
	mInvader.erase(handle);
	
	if (mInvader.size() > 0)
		return;

	spawnWave();
	++mWaveCount;
}

void InvaderGeneral::spawnWave()
{
	ObjectParameter op;

	BFG::u32 invaderRow = static_cast<BFG::u32>(INVADERS_PER_ROW);
	BFG::u32 invaderCol = static_cast<BFG::u32>(INVADERS_PER_COL);

	for (size_t i=0; i < invaderRow; ++i)
	{
		for (size_t j=0; j < invaderCol; ++j)
		{
			std::stringstream ss;
			ss << "Invader No. X:" << i << " Y:" << j;

			op = ObjectParameter();
			op.mHandle = generateHandle();
			op.mName = ss.str();
			op.mType = "Invader";
			op.mLocation.position = v3
			(
				INVADER_MARGIN_X*i - (INVADERS_PER_ROW*INVADER_MARGIN_X)/2,
				INVADER_MARGIN_Y*j - (INVADERS_PER_COL*INVADER_MARGIN_Y)/2 + INVADER_BOTTOM_MARGIN,
				OBJECT_Z_POSITION
			);
			op.mLocation.orientation = INVADER_ORIENTATION;
			op.mLinearVelocity = v3::ZERO;

			mLane->emit(ID::S_CREATE_GO, op);

			mInvader.insert(op.mHandle);
		}
	}
}

void InvaderGeneral::update(quantity<si::time, f32> timeSinceLastFrame)
{
	mLastShot += timeSinceLastFrame;

	if (mLastShot < INVADER_FIRE_INTERVAL)
		return;

	GameHandle player = mEnvironment->find(isPlayer);

	// Player dead?
	if (player == NULL_HANDLE)
		return;

	const v3& playerPosition = mEnvironment->getGoValue<v3>(player, ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);
 
 	f32 lastPlayerInvaderDistance = 0;
 	for (size_t i=0; i < mWaveCount; ++i)
 	{
 		// Handle of Invader and its Distance to the Player
 		std::pair<GameHandle, f32> bestCandidate(NULL_HANDLE, 999999.9f);
 
 		mEnvironment->find
 		(
 			boost::bind
 			(
 				nearestToPlayer,
 				_1,
 				boost::ref(bestCandidate),
 				boost::ref(lastPlayerInvaderDistance),
 				boost::ref(playerPosition)
 			)
 		);
 
 		mLane->emit(ID::GOE_FIRE_ROCKET, Event::Void(), bestCandidate.first);
 
 		lastPlayerInvaderDistance = bestCandidate.second + 0.1f;
 	}
 
 	mLastShot = 0;
}

