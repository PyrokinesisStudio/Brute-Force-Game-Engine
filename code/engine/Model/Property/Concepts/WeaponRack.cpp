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

#include <Model/Property/Concepts/WeaponRack.h>

#include <Base/Logger.h>

#include <Core/GameHandle.h>

#include <Model/Data/ObjectParameters.h>
#include <Model/Environment.h>

namespace BFG {

WeaponRack::WeaponRack(GameObject& owner, PluginId pid) :
Property::Concept(owner, "WeaponRack", pid),
mRocketAmmo(ROCKET_AMMO_START_AMOUNT_FOR_TESTING),
mTarget(NULL_HANDLE)
{
	require("Physical");
	
	subLane()->connectV(ID::GOE_FIRE_ROCKET, this, &WeaponRack::onFireRocket, ownerHandle());
	subLane()->connectV(ID::GOE_FIRE_LASER, this, &WeaponRack::onFireLaser, ownerHandle());
	subLane()->connect(ID::GOE_SET_WEAPON_TARGET, this, &WeaponRack::onSetWeaponTarget, ownerHandle());
	subLane()->connectV(ID::GOE_REINITIALIZE, this, &WeaponRack::onReinitialize, ownerHandle());

	updateGuiAmmo();
}

void WeaponRack::internalUpdate(quantity<si::time, f32> /*timeSinceLastFrame*/)
{}

void WeaponRack::onFireRocket()
{
	// No target?
	if (mTarget == NULL_HANDLE)
		return;
		
	// Out of ammo?
	if (mRocketAmmo < 1)
		return;		

	const f32 spawnDistance = 5.0f;
	const f32 startImpulse = 50.0f;

	--mRocketAmmo;

	Location spawnLocation;
	calculateSpawnLocation(spawnLocation, spawnDistance);
	
	v3 startVelocity;
	calculateVelocity(startImpulse, startVelocity, true);

	static size_t Rocketcount = 0;
	++Rocketcount;
	
	std::stringstream ss;
	ss << "Rocket " << Rocketcount;

	ObjectParameter op;
	op.mHandle = generateHandle();
	op.mName = ss.str();
	op.mType = "Rocket";
	op.mLocation = spawnLocation;
	op.mLinearVelocity = startVelocity;
	
	subLane()->emit(ID::S_CREATE_GO, op);
	subLane()->emit(ID::GOE_AUTONAVIGATE, mTarget, op.mHandle);
	
	updateGuiAmmo();
}

void WeaponRack::onFireLaser()
{
	const float spawnDistance = 15.0f;
	const float startImpulse = 800.0f;	// 200 m/s

	Location spawnLocation;
	calculateSpawnLocation(spawnLocation, spawnDistance);

	v3 startVelocity;
	calculateVelocity(startImpulse, startVelocity, false);

	static size_t LaserCount = 0;
	++LaserCount;
	
	std::stringstream ss;
	ss << "Laser " << LaserCount;

	ObjectParameter op;
	op.mHandle = generateHandle();
	op.mName = ss.str();
	op.mType = "Laser";
	op.mLocation = spawnLocation;
	op.mLinearVelocity = startVelocity;
	
	subLane()->emit(ID::S_CREATE_GO, op);
}

void WeaponRack::onSetWeaponTarget(GameHandle target)
{
	infolog << "New WeaponRack Target: " << mTarget;
	mTarget = target;
}

void WeaponRack::onReinitialize()
{
	mRocketAmmo = ROCKET_AMMO_START_AMOUNT_FOR_TESTING;
	updateGuiAmmo();
}

void WeaponRack::updateGuiAmmo() const
{
	subLane()->emit
	(
		ID::GOE_ROCKET_AMMO,
		mRocketAmmo,
		NULL_HANDLE,
		ownerHandle()
	);
}

void WeaponRack::calculateVelocity(f32 startImpulse,
                                   v3& newVelocity,
                                   bool addShipVelocity) const
{
	const Location& go = getGoValue<Location>(ID::PV_Location, pluginId());

	newVelocity = go.orientation * v3(0, 0, startImpulse);

	if (addShipVelocity)
	{
		const v3& shipVelocity = getGoValue<v3>(ID::PV_Velocity, pluginId());
		newVelocity += shipVelocity;
	}
}

void WeaponRack::calculateSpawnLocation(Location& spawnLocation,
                                        f32 spawnDistance) const
{
	const Location& go = getGoValue<Location>(ID::PV_Location, pluginId());
	
	spawnLocation.position = go.position + go.orientation.zAxis() * spawnDistance;
	spawnLocation.orientation = go.orientation;
}

} // namespace BFG
