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

#include <Model/Property/Concepts/AutoNavigator.h>

#include <Core/Math.h>

#include <Model/Environment.h>
#include <Model/Waypoint.h>
#include <Model/Module.h>

BOOST_UNITS_STATIC_CONSTANT
(
	radian_per_second_squared,
	si::angular_acceleration
);

namespace BFG {

AutoNavigator::AutoNavigator(GameObject& owner, PluginId pid) :
Property::Concept(owner, "AutoNavigator", pid)
{
	initvar(ID::PV_TriggerRadius);

	require("ThrustControl");
	require("Physical");

	subLane()->connect(ID::GOE_AUTONAVIGATE, this, &AutoNavigator::onAutonavigate, ownerHandle());
	subLane()->connect(ID::GOE_VALUE_UPDATED, this, &AutoNavigator::onValueUpdated, ownerHandle());

	assert(ownerHandle() == rootModule() &&
		"AutoNavigator: This code may contain some out-dated assumptions.");
}

AutoNavigator::~AutoNavigator()
{}

void AutoNavigator::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	if (mTargets.empty())
		return;

	mTime = timeSinceLastFrame;

	operate();
}

void AutoNavigator::internalSynchronize()
{
	if (mTargets.empty())
		return;

	subLane()->emit(ID::GOE_CONTROL_PITCH, mRotationFactorPitch, ownerHandle());
	subLane()->emit(ID::GOE_CONTROL_YAW, mRotationFactorYaw, ownerHandle());
	subLane()->emit(ID::GOE_CONTROL_THRUST, mAccelerationFactor, ownerHandle());
}

void AutoNavigator::onAutonavigate(GameHandle target)
{
	mTargets.push_back(target);
}

void AutoNavigator::onValueUpdated(const Property::ValueId& valueId)
{
	if (valueId.mVarId == ID::PV_MaxAngularAcceleration)
		recalculateParameters();
}

void AutoNavigator::operate()
{
	GameHandle targetHandle = *mTargets.begin();

	// Check if target still exists
	if (! environment()->exists(targetHandle))
	{
		mTargets.erase(mTargets.begin());
		return;
	}
	
	const Location& own = getGoValue<Location>(ID::PV_Location, pluginId());
	Location target = environment()->getGoValue<Location>(targetHandle, ID::PV_Location, pluginId());
	
	if (nearEnough(own.position, target.position, mRadius.value()))
	{
		mTargets.erase(mTargets.begin());
		return;
	}

	const v3& targetVelocity = environment()->getGoValue<v3>(targetHandle, ID::PV_Velocity, pluginId());
	const v3& ownVelocity = getGoValue<v3>(ID::PV_Velocity, pluginId());
	
	// use two further frame positions because it takes 2 frames for the value to reach the physics
	v3 targetPositionEx(target.position + targetVelocity * 2.0f * mTime.value());
	v3 ownPositionEx(own.position + ownVelocity * 2.0f * mTime.value());
	
	// \todo use rotation velocity to approximate the orientation in the next frame
	v3 VectorToTarget(unitInverse(own.orientation) * (targetPositionEx - ownPositionEx));

	rotate( rotationTo(v3::UNIT_Z, VectorToTarget) );

	norm(VectorToTarget);

	// if the target lies behind the object, than z of VectorToTarget is negative
	// and the object slows down.
 	accelerate((f32)VectorToTarget.z * mMaxSpeed);
}

void AutoNavigator::rotate(const qv4& rotation)
{
	const v3& currentRotationVel = getGoValue<v3>(ID::PV_RelativeRotationVelocity, pluginId());

	// braking distance in radian
	v3 b = (currentRotationVel * currentRotationVel) / (2.0f * mMaxAngularAcceleration.value());

	// reduce the vibration caused by the time delay of 2 frames by using the braking distance * 3 
	b *= 2.0f;

	quantity<si::plane_angle,f32> pitch = rotation.getPitch(false) * si::radian;
	quantity<si::plane_angle,f32> yaw = rotation.getYaw(false) * si::radian;

	// get the sign
	f32 factor = sign(pitch.value());

	if ((factor * pitch) > (b.x * si::radian))
	{
		if ((factor * pitch) > mOptimalAngle)
			mRotationFactorPitch = 1.0f * factor;
		else
			mRotationFactorPitch = 0.05f * factor;
	}
	else
	{
		mRotationFactorPitch = 0.0f;
	}

	// get the sign
	factor = sign(yaw.value());

	if ((factor * yaw) > (b.y * si::radian))
	{
		if ((factor * yaw) > mOptimalAngle)
			mRotationFactorYaw = 1.0f * factor;
		else
			mRotationFactorYaw = 0.05f * factor;
	}
	else
		mRotationFactorYaw = 0.0f;
}

void AutoNavigator::accelerate(quantity<si::velocity, f32> targetSpeed)
{
	mAccelerationFactor = targetSpeed.value() / mMaxSpeed.value();
}

void AutoNavigator::recalculateParameters()
{
	// GameObject values
	const f32& maa = environment()->getGoValue<f32>
	(
		ownerHandle(),
		ID::PV_MaxAngularAcceleration,
		pluginId()
	);

	mMaxAngularAcceleration = maa * radian_per_second_squared;

	const f32& ms = environment()->getGoValue<f32>
	(
		ownerHandle(),
		ID::PV_MaxSpeed,
		pluginId()
	);
	
	mMaxSpeed = ms * si::meters_per_second;

	// Module values
	ModuleMapT::iterator it = mModules.begin();
	for (; it != mModules.end(); ++it)
	{
		mRadius = value<f32>(ID::PV_TriggerRadius, it->first) * si::meter;
	}
}

} // namespace BFG