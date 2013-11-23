/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#include <Audio/OpenAL/OpenALListener.h>

#ifdef BFG_USE_OPENAL

#include <Audio/Main.h>
#include <Audio/Enums.hh>
#include <Audio/HelperFunctions.h>

namespace BFG {
namespace Audio {

OpenALListener::OpenALListener(Event::Lane& lane) : Listener(lane)
{
	mLane.connect(ID::AE_MASTER_GAIN, this, &OpenALListener::onEventMasterGain);
	mLane.connect(ID::AE_POSITION_PLAYER, this, &OpenALListener::onEventPositionPlayer);
	mLane.connect(ID::AE_ORIENTATION_PLAYER, this, &OpenALListener::onOrientationPlayer);
	mLane.connect(ID::AE_VELOCITY_PLAYER, this, &OpenALListener::onVelocityPlayer);
}

OpenALListener::~OpenALListener()
{}

void OpenALListener::onEventMasterGain(const f32& gain)
{
	alListenerf(AL_GAIN, static_cast<ALfloat>(gain));
	alErrorHandler("OpenALListener::onEventMasterGain", "Error occured calling alListenerf.");
}

void OpenALListener::onEventPositionPlayer(const v3& position)
{
	alListener3f(AL_POSITION, position.x, position.y, position.z);
	alErrorHandler("OpenALListener::onEventPositionPlayer", "Error occured calling alListener3f.");
}

void OpenALListener::onOrientationPlayer(const v3& orientation)
{
	alListener3f(AL_ORIENTATION, orientation.x, orientation.y, orientation.z);
	alErrorHandler("OpenALListener::onOrientationPlayer", "Error occured calling alListener3f.");
}

void OpenALListener::onVelocityPlayer(const v3& velocity)
{
	alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	alErrorHandler("OpenALListener::onVelocityPlayer", "Error occured calling alListener3f.");
}

} // namespace Audio
} // namespace BFG

#endif