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

#ifndef AUDIO_PLAYLIST_H
#define AUDIO_PLAYLIST_H

#include <Audio/AudioModule.h>
#include <Audio/AudioObject.h>

namespace BFG {
namespace Audio {

//! Plays a list of tracks. The playlist starts playing immediatly after creation.
class BFG_AUDIO_API Playlist : public AudioModule
{
	enum State
	{
		INITIAL,
		PLAYING,
		PAUSE,
		FINISHED
	};

public:
	Playlist(const std::vector<std::string>& program,
		     bool repeatAll);

	void pause();
	//! Call it to resume from PAUSE or play the program again if FINISHED.
	//! A call while stated PLAYING is active will be ingored.
	void play();

protected:
	
	//! Will be called if a track is finished.
	void onStreamFinishedForwarded();

private:
	
	void onEventPlay();
	void onEventPause();

	typedef std::vector<boost::shared_ptr<AudioObject> > ProgramT;
	ProgramT mProgram;
	ProgramT::iterator mCurrentTrack;

	State mState;
	bool mRepeatAll;
};

} // namespace Audio
} // namespace BFG

#endif
