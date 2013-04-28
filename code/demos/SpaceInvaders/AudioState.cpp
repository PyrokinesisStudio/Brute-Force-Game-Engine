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

#include <EventSystem/Core/EventLoop.h>
#include <Audio/Main.h>


AudioState::AudioState()
{
	Path p;
	std::vector<std::string> program;
	program.push_back(p.Get(ID::P_SOUND_MUSIC)+"6 Fleet's Arrival.ogg");
	program.push_back(p.Get(ID::P_SOUND_MUSIC)+"02_Deimos - Flottenkommando.ogg");
	program.push_back(p.Get(ID::P_SOUND_MUSIC)+"01_Deimos - Faint Sun.ogg");

	mPlaylist.reset(new Audio::Playlist(program, true));

	Audio::Main::eventLoop()->connect(ID::AE_SOUND_EMITTER_PROCESS_SOUND,
	                                  this,
	                                  &AudioState::audioStateEventHandler);
	Audio::Main::eventLoop()->connect(ID::AE_SOUND_EFFECT,
	                                  this,
	                                  &AudioState::audioStateEventHandler);

	mSoundEffectMap[stringToArray<128>("Explosion_big")] = p.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.wav";
	mSoundEffectMap[stringToArray<128>("Explosion_small")] = p.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.wav";
	mSoundEffectMap[stringToArray<128>("Explosion_medium")] = p.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.wav";
}

AudioState::~AudioState()
{
	Audio::Main::eventLoop()->disconnect(ID::AE_SOUND_EMITTER_PROCESS_SOUND, this);
	Audio::Main::eventLoop()->disconnect(ID::AE_SOUND_EFFECT, this);
}

void AudioState::audioStateEventHandler(Audio::AudioEvent* AE)
{
	switch(AE->getId())
	{
		case ID::AE_SOUND_EMITTER_PROCESS_SOUND:
			mSoundEmitter.processSound(boost::get<std::string>(AE->getData()));
			break;
		case ID::AE_SOUND_EFFECT:
			onSoundEffect(AE->getData());			
			break;
		default:
			throw std::logic_error("AudioState::eventHandler: received unhandled event!");
	}
}

void AudioState::onSoundEffect(const Audio::AudioPayloadT& payload)
{
	CharArray128T effect = boost::get<CharArray128T>(payload);

	SoundEffectMapT::iterator it;
	
	it = mSoundEffectMap.find(effect);

	mSoundEmitter.processSound(it->second);
}

