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
	mLane(lane),
	mEnvironment(environment),
	mLastShot(0),
	mWaveCount(0)
{
	spawnWave();
	++mWaveCount;
}

void InvaderGeneral::spawnWave()
{
	ObjectParameter op;

	for (size_t i=0; i < (size_t) INVADERS_PER_ROW; ++i)
	{
		for (size_t j=0; j < (size_t) INVADERS_PER_COL; ++j)
		{
			std::stringstream ss;
			ss << "Invader No. X:" << i << " Y:" << j;

			boost::shared_ptr<GameObject> invader;

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
		}
	}
}

void InvaderGeneral::update(quantity<si::time, f32> timeSinceLastFrame)
{
	mLastShot += timeSinceLastFrame;

	if (mLastShot < INVADER_FIRE_INTERVAL)
		return;

	if (mEnvironment->find(isInvader) == NULL_HANDLE)
	{
		spawnWave();
		++mWaveCount;
		return;
	}

	GameHandle player = mEnvironment->find(isPlayer);

	// Player dead?
	if (player == NULL_HANDLE)
		return;

	const Location& playerLoc = mEnvironment->getGoValue<Location>(player, ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

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
				boost::ref(playerLoc.position)
			)
		);

		mLane->emit(ID::GOE_FIRE_ROCKET, Event::Void(), bestCandidate.first);

		lastPlayerInvaderDistance = bestCandidate.second + 0.1f;
	}

	mLastShot = 0;
}

