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

#ifndef BFG_AUDIO_OPEN_AL_LISTENER_H
#define BFG_AUDIO_OPEN_AL_LISTENER_H

#include <Audio/Listener.h>

#ifdef BFG_USE_OPENAL

#include <al.h>
#include <alc.h>

namespace BFG {
namespace Audio {

//! Provides the listener features in OpenAL context.
class BFG_AUDIO_API OpenALListener : public Listener
{
public:

	OpenALListener();
	~OpenALListener();

private:

	void eventHandler(AudioEvent* AE);

	void onEventMasterGain(const AudioPayloadT& payload);
	
	void onVelocityPlayer(const AudioPayloadT& payload);
	void onOrientationPlayer(const AudioPayloadT& payload);
	void onEventPositionPlayer(const AudioPayloadT& payload);
};

// Concrete creation method for OpenAL listener instance.
boost::shared_ptr<Listener> createListener() { return boost::shared_ptr<Listener>(new OpenALListener()); }


} // namespace Audio
} // namespace BFG

#endif // BFG_USE_OPENAL
#endif
