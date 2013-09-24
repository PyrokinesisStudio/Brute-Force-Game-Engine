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

#ifndef AUDIO_TEST_LOADER_H_
#define AUDIO_TEST_LOADER_H_

#include <Audio/Audio.h>
#include <Event/Event.h>
#include <tests/AudioTest/functions.h>

namespace BFG
{

typedef void (*pFunc)();

class AudioTestLoader
{

public:

	AudioTestLoader() : mLane(mSynchronizer, 100)
	{
		startEventLoop();
	}

	~AudioTestLoader()
	{
		mSynchronizer.finish();
	}

	static void registerTestFunction(pFunc testFunction)
	{
		mTestFunction = testFunction;
	}

private:

	void startEventLoop()
	{
		using namespace BFG;

		mLane.addEntry<Audio::Main>();
		mLane.connectLoop(this, &AudioTestLoader::loopEventHandler);
		mSynchronizer.start();
		
		mTestFunction();
	}

	void loopEventHandler(const Event::TickData tickData)
	{
		//boost::this_thread::sleep(boost::posix_time::millisec(2));
	}

	static pFunc mTestFunction;
	Audio::Main mAudioMain;

	Event::Synchronizer mSynchronizer;
	Event::Lane mLane;
};

}

#endif
