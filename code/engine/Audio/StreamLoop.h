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

#ifndef AUDIO_STREAM_LOOP
#define AUDIO_STREAM_LOOP

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <Core/Types.h>

#include <Audio/Defines.h>
#include <Audio/Stream.h>

namespace BFG {
namespace Audio {

//! This class is the owner of the stream thread. This class does not stream the
//! data by itself. It just provides an interface to register and unregister
//! streams to the loop (the thread).
//! This class has three containers:
//! 1. One for new streams. Those are streams which are waiting for being put on the loop.
//! 2. One for the streams which are on the loop. Those are streams which become processed at the moment.
//! 3. One for the finished streams. Streams which must be deleted from the loop.
//! It is very important to understand that the moment when an audio track has
//! finished streaming is different to the moment when an audio track has been
//! finished playing!  The AudioObject knows when a track is finished. It will
//! delete its stream by calling removeMyStream() This is the reason why we need
//! three containers and a thread.
//! If using OpenAL:
//! The processing of the audio data is deep within OpenAL. Its thread-safety is
//! provided there.  OpenAL does not provide the streaming.
class BFG_AUDIO_API StreamLoop
{
public:
	StreamLoop();
	~StreamLoop();

	typedef u32 StreamHandleT;

	//! Registers a Stream to the loop.
	StreamHandleT driveMyStream(boost::shared_ptr<Stream> stream);

	//! Unregisters a Stream from the loop.
	void removeMyStream(StreamHandleT streamHandle);

private:
	bool mIsRunning;
	
	//! The stream loop provides its own thread which processes the stream:
	//! stream->nextStreamStep();
	//! The AudioObject, in the other thread, recognizes if the sound has
	//! been played to the end. Remember, streaming and processing audio
	//! data is not the same! Streaming is done to load the data successive
	//! from the audio file format. Processing audio data is done to play
	//! this data on on your audio device.
	boost::thread mThread;
	boost::mutex mRefreshMutex;
	
	//! The thread loop.
	void onStreaming();
	
	void init(const std::vector<std::string>& filelist);
	void removeFinishedStreams();
	void removeStream(StreamHandleT streamHandle);

	typedef std::map<StreamHandleT, boost::shared_ptr<Stream> > StreamsMapT;
	typedef std::vector<StreamHandleT> FinishedStreamsT;
	
	StreamHandleT mStreamHandleCounter;

	StreamsMapT mStreamsOnLoop;
	FinishedStreamsT mFinishedStreams;
	StreamsMapT mNewStreams;
};

} // namespace Audio
} // namespace BFG

#endif

