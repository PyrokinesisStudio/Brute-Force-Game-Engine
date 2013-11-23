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

#include <Audio/Modules/SoundEmitter.h>
#include <Audio/Main.h>

namespace BFG {
namespace Audio {

SoundEmitter::SoundEmitter():
    mState(PLAYING),
    mIdCounter(0)
{
}

void SoundEmitter::play()
{
	if (mState == PLAYING)
		return;

	SoundQueueT::iterator it = mSoundQueue.begin();

	for (; it != mSoundQueue.end(); ++it)
		it->second->play();

	mState = PLAYING;
}

void SoundEmitter::pause()
{
	SoundQueueT::iterator it = mSoundQueue.begin();

	for (; it != mSoundQueue.end(); ++it)
		it->second->pause();

	mState = PAUSE;
}

void SoundEmitter::processSound(const std::string &name)
{
	if (mState != PLAYING)
		return;
	
	boost::mutex::scoped_lock lock(mMutex);

	++mIdCounter;

	boost::shared_ptr<SoundHandle> handle;
	handle.reset(new SoundHandle(*this, mIdCounter));

	mSoundHandles[mIdCounter] = handle;

	boost::function<void(void)> onFinishedCallback = boost::bind
	(
		&SoundHandle::onStreamFinishedForwarded,
		&(*handle)
	);

	boost::shared_ptr<AudioObject> audioObject = createAudioObject(name, mStreamLoop, onFinishedCallback);
	mSoundQueue[mIdCounter] = audioObject;
	audioObject->play();
}

void SoundEmitter::soundFinished(int id)
{
	boost::mutex::scoped_lock lock(mMutex);

	SoundQueueT::iterator itSounds = mSoundQueue.find(id);
	mSoundQueue.erase(itSounds);

	HandlesT::iterator itHandles = mSoundHandles.find(id);
	mSoundHandles.erase(itHandles);
}

void SoundEmitter::volume(f32 gain)
{
	SoundQueueT::iterator ao = mSoundQueue.begin();

	for (; ao != mSoundQueue.end(); ++ao)
		ao->second->volume(gain);
}

} // namespace Audio
} // namespace BFG
