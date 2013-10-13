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

#ifndef AUDIO_OPEN_AL_STREAM_H_
#define AUDIO_OPEN_AL_STREAM_H_

#include <Audio/Defines.h>

#ifdef BFG_USE_OPENAL

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/scoped_array.hpp>

#include <al.h>

#include <Core/Types.h>
#include <Audio/Stream.h>


namespace BFG {
namespace Audio {

class AudioFile;

//! Provides the streaming of audio data with OpenAL
class BFG_AUDIO_API OpenALStream : public Stream
{

public:

	OpenALStream(boost::shared_ptr<AudioFile> file,
		         boost::function<void (void)> onStreamFinished,
	             ALuint sourceId);

	~OpenALStream();

	//! Refills the stream buffers and calls the stream finished callback.
	//! This method will usually be called only from StreamLoop.
	void nextStreamStep();

private:
	void preload();

	const s32 mNUM_BUFFER;
	ALuint mSourceId;
	bool mFinished;
	ALfloat mGain;

	boost::scoped_array<ALuint> mBufferIds;
};

} // namespace Audio
} // namespace BFG

#endif // BFG_USE_OPENAL
#endif
