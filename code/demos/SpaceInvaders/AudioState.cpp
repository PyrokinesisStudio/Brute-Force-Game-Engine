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

#include "AudioState.h"
#include <Core/Path.h>
#include <Core/XmlFileHandleFactory.h>
#include <Event/Event.h>

#include <Audio/Audio.h>

AudioState::AudioState(Event::SubLanePtr subLane) :
	mSubLane(subLane)
{
	Audio::PlaylistXml musicFile("default/Music.xml");
	
	mPlaylist.reset(new Audio::Playlist(musicFile));
	
	mSubLane->connect(ID::AE_SOUND_EMITTER_PROCESS_SOUND, this, &AudioState::onSoundEmitterProcessSound);
	mSubLane->connect(ID::AE_SOUND_EFFECT, this, &AudioState::onSoundEffect);
	
	Path p;
	mSoundEffectMap["Explosion_big"] = p.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.ogg";
	mSoundEffectMap["Explosion_small"] = p.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.ogg";
	mSoundEffectMap["Explosion_medium"] = p.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.ogg";
}

AudioState::~AudioState()
{

}

void AudioState::onSoundEmitterProcessSound(const std::string& effect)
{
	mSoundEmitter.processSound(effect);
}

void AudioState::onSoundEffect(const std::string& effect)
{
	SoundEffectMapT::iterator it = mSoundEffectMap.find(effect);
	mSoundEmitter.processSound(it->second);
}
