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


#ifndef SI_AUDIO_STATE_H
#define SI_AUDIO_STATE_H

#include <Base/LibraryMainBase.h>

#include <Event/Event.h>

#include <Audio/Modules/Playlist.h>
#include <Audio/Modules/SoundEmitter.h>

using namespace BFG;

class AudioState
{
public:
	AudioState(Event::SubLanePtr subLane);
	~AudioState();

private:
	void onSoundEffect(const std::string& effect);
	void onSoundEmitterProcessSound(const std::string& effect);

	boost::scoped_ptr<Audio::Playlist> mPlaylist;
	Audio::SoundEmitter mSoundEmitter;
	
	typedef std::map<std::string, std::string> SoundEffectMapT;
	SoundEffectMapT mSoundEffectMap;
	Event::SubLanePtr mSubLane;
};

#endif
