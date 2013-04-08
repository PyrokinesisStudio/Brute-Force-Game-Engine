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

#ifndef BFG_AUDIO_OPEN_AL_INIT_H
#define BFG_AUDIO_OPEN_AL_INIT_H

#include <Audio/Init.h>

#ifdef BFG_USE_OPENAL

#include <boost/shared_ptr.hpp>

#include <al.h>
#include <alc.h>

#include <Audio/Defines.h>


namespace BFG {
namespace Audio {

//! Initialize OpenAL
class BFG_AUDIO_API OpenALInit : public Init
{
public:
	OpenALInit();
	~OpenALInit();

private:
	void init();

	ALCdevice* mDevice;
	ALCcontext* mContext;
};

//! Contrete creation method for a OpenAL instance.
boost::shared_ptr<Init> createInit() { return boost::shared_ptr<Init>(new OpenALInit()); }

} // namespace Audio
} // namespace BFG

#endif // BFG_USE_OPENAL
#endif
