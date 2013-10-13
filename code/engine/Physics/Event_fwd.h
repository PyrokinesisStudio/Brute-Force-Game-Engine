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

#ifndef PHYSICS_EVENT_FWD_H
#define PHYSICS_EVENT_FWD_H

#include <boost/tuple/tuple.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/mass.hpp>

#include <Core/ExternalTypes.h>
#include <Core/GameHandle.h>
#include <Core/Location.h>

#include <Physics/Enums.hh>

namespace BFG {
namespace Physics {

using namespace boost::units;

typedef boost::tuple
<
	u32,                      // Timestamp
	u16,                      // Age of data
	v3                        // Abs. Position
> InterpolationDataV3;

typedef boost::tuple
<
	u32,                      // Timestamp
	u16,                      // Age of data
	qv4                       // Orientation
> InterpolationDataQv4;

typedef boost::tuple
<
	v3,                       // Position
	qv4,                      // Orientation
	v3,                       // Velocity
	v3,                       // Relative Velocity
	v3,                       // Rotation Velocity
	v3,                       // Relative Rotation Velocity
	f32,                      // Total Mass
	m3x3                      // Inertia
> FullSyncData;

typedef boost::tuple
<
	v3,                  // Absolute
	v3                   // Relative
> VelocityComposite;

typedef boost::tuple
<
	GameHandle,
	Location
> ObjectCreationParams;

struct ModuleCreationParams
{
	ModuleCreationParams() :
	mGoHandle(NULL_HANDLE),
	mModuleHandle(NULL_HANDLE),
	mDensity(1.0f),
	mCollisionMode(ID::CM_Disabled)
	{}

	ModuleCreationParams(
		GameHandle gameObjectHandle,
		GameHandle moduleHandle,
		const std::string& meshName,
		f32 density,
		ID::CollisionMode collisionMode = ID::CM_Standard,
		v3 position = v3::ZERO,
		qv4 orientation = qv4::IDENTITY,
		v3 velocity = v3::ZERO,
		v3 rotationVelocity = v3::ZERO) :
	mGoHandle(gameObjectHandle),
	mModuleHandle(moduleHandle),
	mMeshName(meshName),
	mDensity(density),
	mCollisionMode(collisionMode),
	mPosition(position),
	mOrientation(orientation),
	mVelocity(velocity),
	mRotationVelocity(rotationVelocity)
	{}


	GameHandle mGoHandle;             // Handle of PhysicsObject
	GameHandle mModuleHandle;         // Module Handle
	std::string mMeshName;            // Mesh Name (e.g. "Cube.mesh")
	f32 mDensity;                     // Density
	ID::CollisionMode mCollisionMode;
	v3 mPosition;                     // Module Position
	qv4 mOrientation;                 // Module Orientation
	v3 mVelocity;                     // Start Velocity
	v3 mRotationVelocity;             // Start Rotation Velocity
};

typedef boost::tuple
<
	GameHandle,          // Handle of PhysicsObject
	GameHandle           // Module Handle
> ModuleRemovalParams;

typedef boost::tuple
<
	GameHandle,          // Parent
	GameHandle,          // Child
	v3,                  // Offset Position
	qv4                  // Offset Orientation
> ObjectAttachmentParams;

typedef boost::tuple
<
	GameHandle,          // Own Module
	GameHandle,          // Other Module
	f32                  // Penetration Depth
> ModulePenetration;

} // namespace Physics
} // namespace BFG

#endif