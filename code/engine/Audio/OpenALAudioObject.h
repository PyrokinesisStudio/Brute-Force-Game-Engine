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

#ifndef OPEN_AL_AUDIO_OBJECT_H
#define OPEN_AL_AUDIO_OBJECT_H

#include <Audio/AudioObject.h>

#ifdef BFG_USE_OPENAL

#include <al.h>


namespace BFG {
namespace Audio {

class StreamLoop;

class BFG_AUDIO_API OpenALAudioObject : public AudioObject
{

public:
	OpenALAudioObject(std::string audioName, 
		              boost::shared_ptr<StreamLoop> streamLoop,
					  boost::function<void (void)> onFinishedForward = 0,
					  f32 initalGain = 1.0f);
	
	~OpenALAudioObject();

	void play();
	void pause();
	void stop();

	void volume(f32 gain);

private:

	void onStreamFinished();
	void careOfSource();

	ALuint mSourceId;
};

boost::shared_ptr<AudioObject> createAudioObject(std::string audioName, 
	                                             boost::shared_ptr<StreamLoop> streamLoop,
												 boost::function<void (void)> onFinishedForward)
{
	return boost::shared_ptr<AudioObject>(new OpenALAudioObject(audioName, streamLoop, onFinishedForward));
}

} // namespace Audio
} // namespace BFG

#endif // BFG_USE_OPENAL
#endif