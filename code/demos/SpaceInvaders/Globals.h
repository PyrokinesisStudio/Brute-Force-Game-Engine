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

#ifndef SI_GLOBALS_H
#define SI_GLOBALS_H

#include <boost/units/systems/si/velocity.hpp>
#include <boost/units/systems/si/length.hpp>

#include <OgreSceneManager.h>
#include <OgreParticleSystem.h>

#include <Core/Types.h>
#include <Core/Math.h>

#include <Core/GameHandle.h>

#include <Model/Environment.h>
#include <Model/GameObject.h>


using namespace BFG;

const f32 OBJECT_Z_POSITION = 80.0f;
const f32 DISTANCE_TO_WALL = 40.0f;
const f32 NEGATIVE_SHIP_Y_POSITION = 35.0f;

const f32 INVADER_MARGIN_X = 7.0f;
const f32 INVADER_MARGIN_Y = 8.0f;
const f32 INVADER_BOTTOM_MARGIN = 25.0f;
const f32 INVADERS_PER_ROW = 11.0f;
const f32 INVADERS_PER_COL = 5.0f;

const f32 SHIP_SPEED_MULTIPLIER = 25.0f;

const ValueId::VarIdT ID_PROJECTILE_SPEED          = 1;
const ValueId::VarIdT ID_PROJECTILE_SPAWN_DISTANCE = 2;

const v3 INVADER_VELOCITY = v3(4.0f, 0.2f, 0.0f);
const f32 INVADER_TURNING_DISTANCE = (INVADER_MARGIN_X * INVADERS_PER_ROW) / 2;
const quantity<si::time, f32> INVADER_FIRE_INTERVAL = 5.0f * si::seconds;

const qv4 INVADER_ORIENTATION(0, 0, 0.707107f, -0.707107f);

const s32 A_SHIP_AXIS_Y = 10000;
const s32 A_SHIP_FIRE   = 10001;
const s32 A_QUIT        = 10002;
const s32 A_FPS         = 10003;
const s32 A_CONSOLE     = 10004;



bool nearestToPlayer(boost::shared_ptr<BFG::GameObject> go,
                     std::pair<GameHandle, f32>& bestCandidate,
                     const f32& minDistanceToPlayer,
                     const v3& playerPosition);

bool isPlayer(boost::shared_ptr<BFG::GameObject> go);
bool isInvader(boost::shared_ptr<BFG::GameObject> go);

void fireParticles(Ogre::ParticleSystem* particleSystem,
				   Ogre::SceneNode* sceneNode,
				   f32 startTime,
				   f32 endTime,
				   f32 elapsedTime);


#endif