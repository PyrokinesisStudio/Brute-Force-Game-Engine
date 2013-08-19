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

#include "Cannon.h"

#include <Core/Path.h>
#include <Audio/Enums.hh>
#include <Model/Environment.h>
#include <Model/Data/GameObjectFactory.h>
#include <Model/Sector.h>

#include "Globals.h"


Cannon::Cannon(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "Cannon", pid),
	mAutoRocketAmmo(0)
{
	require("Physical");

	// TODO having a path here is not beautiful.
	Path path;
	mLaserSound = path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav";

	initvar(ID_PROJECTILE_SPEED);
	initvar(ID_PROJECTILE_SPAWN_DISTANCE);

	mSubLane->connectV(ID::GOE_FIRE_ROCKET, this, &Cannon::onFireRocket, ownerHandle());
	mSubLane->connect(ID::GOE_POWERUP, this, &Cannon::onPowerUp, ownerHandle());
}

void Cannon::onFireRocket()
{
	if (mAutoRocketAmmo > 0)
	{
		fireRocket(true);
		--mAutoRocketAmmo;
	}
	else
	{
		fireRocket(false);
	}
}

void Cannon::onPowerUp(s32 ammo)
{
	mAutoRocketAmmo += ammo;
}

void Cannon::fireRocket(bool autoRocket)
{
	std::vector<GameHandle> targets = environment()->find_all(isInvader);

	if (autoRocket && targets.empty())
		return;

	const Location& go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

	const f32& projectileSpeed = value<f32>(ID_PROJECTILE_SPEED, rootModule());
	const f32& projectileSpawnDistance = value<f32>(ID_PROJECTILE_SPAWN_DISTANCE, rootModule());

	Location spawnLocation;
	spawnLocation.position = go.position + go.orientation.zAxis() * projectileSpawnDistance;
	spawnLocation.orientation = go.orientation;

	// Make Name
	static size_t count = 0;
	++count;
	
	std::stringstream ss;
	ss << "Projectile " << count;

	// Make Configuration
	ObjectParameter op;
	op.mHandle = generateHandle();
	op.mName = ss.str();
	
	if (autoRocket) 
		op.mType = "AutoProjectile";
	else
		op.mType = "Projectile";
	
	op.mLocation = spawnLocation;
	op.mLinearVelocity = v3(projectileSpeed) * go.orientation.zAxis();

	mSubLane->emit(ID::S_CREATE_GO, op);
	mSubLane->emit(ID::AE_SOUND_EMITTER_PROCESS_SOUND, mLaserSound);

	if (autoRocket)
	{
		GameHandle randomInvader = targets[rand() % targets.size()];
		mSubLane->emit(ID::GOE_AUTONAVIGATE, randomInvader, op.mHandle);
	}
}

