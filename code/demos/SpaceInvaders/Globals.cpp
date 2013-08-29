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

#include "Globals.h"


bool nearestToPlayer(boost::shared_ptr<BFG::GameObject> go,
                     std::pair<GameHandle, f32>& bestCandidate,
                     const f32& minDistanceToPlayer,
                     const v3& playerPosition)
{
	if (! go->satisfiesRequirement("InvaderControl"))
		return false;

	const v3& invaderPosition = go->getValue<v3>(ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);
	
	f32 invaderPlayerDistance = BFG::distance(invaderPosition, playerPosition);
	
	if (invaderPlayerDistance < bestCandidate.second &&
	    invaderPlayerDistance > minDistanceToPlayer)
		bestCandidate = std::make_pair(go->getHandle(), invaderPlayerDistance);

	// We need to iterate through all invaders
	return false;
}


bool isPlayer(boost::shared_ptr<BFG::GameObject> go)
{
	return go->satisfiesRequirement("ShipControl");
}


bool isInvader(boost::shared_ptr<BFG::GameObject> go)
{
	return go->satisfiesRequirement("InvaderControl");
}


void fireParticles(Ogre::ParticleSystem* particleSystem,
				   Ogre::SceneNode* sceneNode,
				   f32 startTime,
				   f32 endTime,
				   f32 elapsedTime)
{
	if ((elapsedTime > startTime) && (elapsedTime < endTime))
	{
		if(!particleSystem->isAttached())
			sceneNode->attachObject(particleSystem);
		return;
	}
	if (elapsedTime > endTime)
	{
		if (particleSystem->isAttached())
			sceneNode->detachObject(particleSystem);	
		return;
	}
}
