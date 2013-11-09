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

#ifndef AUDIO_SOUND_EMITTER_H
#define AUDIO_SOUND_EMITTER_H

#include <boost/thread/mutex.hpp>

#include <Core/Types.h>
#include <Audio/AudioModule.h>
#include <Audio/AudioObject.h>

namespace BFG {
namespace Audio {

//! This higher level module provides a "fire and forget" feature for playing sounds.
//! It queues the audioObjects until they are finished.
class BFG_AUDIO_API SoundEmitter : public AudioModule
{
	enum State
	{
		PLAYING,
        PAUSE
    };


	//! The soundhandle provides an ID and a callback.
	//! If the sound is finished the callback removes the audioObject out of the queue.
    class BFG_AUDIO_API SoundHandle
    {

    public:

        SoundHandle(SoundEmitter& soundEmitter,
                    u32 id) :
            mSoundEmitter(soundEmitter),
            mId(id)
        {}

        void onStreamFinishedForwarded()
        {
			mSoundEmitter.soundFinished(mId);
        }

    private:

        SoundEmitter& mSoundEmitter;
        u32 mId;
    };

public:

    SoundEmitter();

    void processSound(const std::string& name);
    void soundFinished(int id);
	
	//! Just to resume from PAUSE.
	void play();
	void pause();

	void volume(f32 gain);

protected:
	
	void onStreamFinishedForwarded() { /*NOT USED FOR THIS MODULE*/ }

private:

    typedef std::map<u32, boost::shared_ptr<AudioObject> > SoundQueueT;
    typedef std::map<u32, boost::shared_ptr<SoundHandle> > HandlesT;
	SoundQueueT mSoundQueue;
    HandlesT mSoundHandles;

    State mState;
    u32 mIdCounter;

    boost::mutex mMutex;
};

} // namespace Audio
} // namespace BFG

#endif
