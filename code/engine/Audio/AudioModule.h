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

#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

#include <Audio/Defines.h>
#include <boost/shared_ptr.hpp>
#include <Audio/StreamLoop.h>

namespace BFG {
namespace Audio {

//! Abstract class for higher level audio modules.
//! Every higher level audio feature should derive from this class.
//! Every instance of AudioModule owns it own StreamLoop and every StreamLoop has its own working thread.
class BFG_AUDIO_API AudioModule
{

public:
	AudioModule() { mStreamLoop.reset(new StreamLoop()); }

	virtual void volume(f32 gain) = 0;

protected:
	//! Callback forwarder to recognize the end of a stream. This is nice if something should happen after
	//! the sound is finished.
	virtual void onStreamFinishedForwarded() = 0;
	
	//! Own streaming thread.
	boost::shared_ptr<StreamLoop> mStreamLoop;
};

} // namespace Audio
} // namespace BFG

#endif
