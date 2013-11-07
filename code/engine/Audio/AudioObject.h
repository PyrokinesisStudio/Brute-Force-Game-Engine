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

#ifndef AUDIO_OBJECT_H
#define AUDIO_OBJECT_H

#include <Audio/Defines.h>
#include <boost/shared_ptr.hpp>
#include <Audio/StreamLoop.h>

namespace BFG {
namespace Audio {


class BFG_AUDIO_API AudioObject
{

public:
	AudioObject(std::string audioName, 
		        boost::shared_ptr<StreamLoop> streamLoop,
                boost::function<void (void)> onFinishedForward = 0,
				f32 initalGain = 1.0f): 
		mAudioName(audioName),
		mStreamLoop(streamLoop),
		mStreamHandle(0),
        mForwardCallback(onFinishedForward),
		mGain(initalGain)
	{}
	
	~AudioObject() {}

	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
	
	virtual void volume(f32 gain) = 0;

protected:
	virtual void onStreamFinished() = 0;

	std::string mAudioName;
	boost::shared_ptr<StreamLoop> mStreamLoop;
	StreamLoop::StreamHandleT mStreamHandle;
	boost::function<void (void)> mForwardCallback;
	f32 mGain;
};

boost::shared_ptr<AudioObject>	BFG_AUDIO_API createAudioObject(std::string audioName, 
								                                boost::shared_ptr<StreamLoop> streamLoop,
								                                boost::function<void (void)> onFinishedForward = 0);

} // namespace Audio
} // namespace BFG

#endif
