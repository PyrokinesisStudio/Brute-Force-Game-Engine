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

#ifndef POWER_UP_EFFECT_H
#define POWER_UP_EFFECT_H

#include <Core/Math.h>
#include <Core/Types.h>
#include <Core/GameHandle.h>

#include <View/Effect.h>

using namespace BFG;

class PowerupEffect : public View::Effect
{
public:
	PowerupEffect(const v3& position, f32 intensity);
	~PowerupEffect();

	bool frameStarted(const Ogre::FrameEvent& evt);

private:
	v3 mPosition;
	f32 mIntensity;
	GameHandle mHandle;
	Ogre::SceneNode* mNode;
	Ogre::ParticleSystem* mParticleSystem;
	f32 mElapsedTime;
};

#endif