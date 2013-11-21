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

#include <Audio/OpenALInit.h>

#ifdef BFG_USE_OPENAL

#include <Base/Logger.h>
#include <Audio/HelperFunctions.h>

namespace BFG {
namespace Audio {

OpenALSoundDevice::OpenALSoundDevice()
{
	init();
}

OpenALSoundDevice::~OpenALSoundDevice()
{
	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);

	mContext = NULL;
	mDevice = NULL;
}

void OpenALSoundDevice::init()
{
	mDevice = alcOpenDevice(0); // 0 is default device.

	if (!mDevice)
	{
		throw std::logic_error("Open audio device failed!");
	}
	else
		infolog << "OpenAl device created";

	mContext = alcCreateContext(mDevice, 0);

	if (!mContext)
		throw std::logic_error("Creation of AlContext for level failed! ALerror: "+stringifyAlError(alGetError()));
	else
		infolog << "Al context created";

	if (!alcMakeContextCurrent(mContext))
		throw std::logic_error("Making alContext current failed at Level::load. ALError: "+stringifyAlError(alGetError()));
	else
		infolog << "Set Al context current.";
}

} // namespace Audio
} // namespace BFG

#endif
