/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

#include <Model/Property/Concepts/LevelOfDetail.h>

#include <Core/Math.h>
#include <Model/Data/LevelOfDetailFactory.h>
#include <Model/Environment.h>

namespace BFG {

LevelOfDetail::LevelOfDetail(GameObject& owner, PluginId pid) :
Property::Concept(owner, "LevelOfDetail", pid)
{
	requiredPvInitialized(ID::PV_LoD_Variant);
	requiredPvInitialized(ID::PV_LoD);

	require("Physical");	

	mLodAlgorithm = LevelOfDetailFactory::instance()->mLodDefault;
}

void LevelOfDetail::internalUpdate(TimeT timeSinceLastFrame)
{
	if (owner().satisfiesRequirement("Camera"))
		return;

	if (!owner().isActivated())
		return;
	
	const u32 lodType = getGoValue<u32>(ID::PV_LoD_Variant, pluginId());

	switch (lodType)
	{
		case ID::PV_LoD_As_Initialized:
			break;
		case ID::PV_LoD_Default:
			setGoValue(ID::PV_LoD, pluginId(), calculateDefaultLoD());
			break;
	}

	mSubLane->emit(ID::GOE_UPDATE_LOD, getGoValue<u32>(ID::PV_LoD, pluginId()));
}

u32 LevelOfDetail::calculateDefaultLoD()
{
	v3 position = getGoValue<v3>(ID::PV_Position, pluginId());
	qv4 orientation = getGoValue<qv4>(ID::PV_Orientation, pluginId());
	
	GameHandle camera;
	
	try 
	{
		camera = environment()->find(isCamera);
	}
	catch (std::exception&)
	{
		camera = NULL_HANDLE;
	}
	
	v3 cameraPosition = v3::UNIT_Z;
	
	if (camera != NULL_HANDLE)
		cameraPosition= environment()->getGoValue<v3>(camera, ID::PV_Position, pluginId());

	v3 velocity = getGoValue<v3>(ID::PV_Velocity, pluginId());

	return mLodAlgorithm.lock()->get(position, 
	                                 orientation, 
	                                 cameraPosition, 
	                                 length(velocity), 
	                                 0.0f,  // not implemented yet
	                                 0.0f); // not implemented yet
}

}