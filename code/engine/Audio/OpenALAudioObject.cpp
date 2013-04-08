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

#include <Audio/OpenALAudioObject.h>

#ifdef BFG_USE_OPENAL

#include <Base/Logger.h>

#include <Audio/OpenALStream.h>
#include <Audio/FileFactory.h>
#include <Audio/HelperFunctions.h>


namespace BFG {
namespace Audio {

OpenALAudioObject::OpenALAudioObject(std::string audioName, 
	                                 boost::shared_ptr<StreamLoop> streamLoop,
									 boost::function<void (void)> onFinishedForward): 
	AudioObject(audioName, streamLoop, onFinishedForward),
	mSourceId(0)
	{
	}

	OpenALAudioObject::~OpenALAudioObject() 
	{
		if (mSourceId)
		{
			stop();
		}
	}

	void OpenALAudioObject::play()
	{
		careOfSource();

		ALint sourceState;
		alGetSourcei(mSourceId, AL_SOURCE_STATE, &sourceState);
		alErrorHandler("OpenALAudioObject::play", "Error occured calling alGetSourcei");
		
		switch (sourceState)
		{
			case AL_PLAYING:
				return;

			case AL_PAUSED:
				break;

			case AL_INITIAL:
			{
				boost::function<void(void)> onFinishedCallback = boost::bind
				(
					&OpenALAudioObject::onStreamFinished,
					this
				);
				
				boost::shared_ptr<Stream> stream
				(
					new OpenALStream
					(
						createFile(mAudioName),
						onFinishedCallback, 
						mSourceId
					)
				);
				
				mStreamHandle = mStreamLoop->driveMyStream(stream);
				
				break;
			}

			default:
				return;
		}
		
		alSourcePlay(mSourceId);
		alErrorHandler("OpenALAudioObject::play", "Error occured calling alSourcePlay.");
	}

	void OpenALAudioObject::pause()
	{
		if (mSourceId)
		{
			alSourcePause(mSourceId);
			alErrorHandler("OpenALAudioObject::pause", "Error occured calling alSourcePause.");

		}
	}

	void OpenALAudioObject::stop()
	{
		if (mSourceId)
		{
			alSourceStop(mSourceId);
			alErrorHandler("OpenALAudioObject::stop", "Error occured calling alSourceStop.");

			mStreamLoop->removeMyStream(mStreamHandle);
			mStreamHandle = 0;
			
			alDeleteSources(1, &mSourceId);
			alErrorHandler("OpenALAudioObject::stop", "Error occured calling alDeleteSources.");
			mSourceId = 0;
		}
	}

	void OpenALAudioObject::careOfSource()
	{
		if (!mSourceId)
		{
			alGenSources(1, &mSourceId);
			alErrorHandler("OpenALAudioObject::careOfSource", "Error occured calling alGenSources.");
			
			if (!alIsSource(mSourceId))
				throw std::logic_error("Not a valid source ID at OpenALAudioObject::careOfSource().");
		}
	}

	void OpenALAudioObject::onStreamFinished()
	{
		stop();

		if (mForwardCallback)
			mForwardCallback();
	}

} // namespace Audio
} // namespace BFG

#endif
