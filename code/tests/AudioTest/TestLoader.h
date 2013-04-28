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
#include <tests/AudioTest/functions.h>

namespace BFG
{

typedef void (*pFunc)();

class AudioTestLoader
{

public:

	AudioTestLoader()
	{
		mEventLoop = new EventLoop(true);
		startEventLoop();
	}

	~AudioTestLoader()
	{
		mEventLoop->setExitFlag();
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		delete mEventLoop;
		mEventLoop = NULL;
	}

	static void registerTestFunction(pFunc testFunction)
	{
		mTestFunction = testFunction;
	}

private:

	static void * EventLoopEntryPoint(void * iPointer)
	{
		mTestFunction();
		((EventLoop*)iPointer)->setExitFlag();
		return 0;
	}

	void startEventLoop()
	{
		using namespace BFG;

		mEventLoop->addEntryPoint(mAudioMain.entryPoint());
		mEventLoop->addEntryPoint(new Base::CEntryPoint(EventLoopEntryPoint));
		mEventLoop->registerLoopEventListener(this, &AudioTestLoader::loopEventHandler);

		mEventLoop->run();
	}

	void loopEventHandler(LoopEvent* loopEvent)
	{
		//boost::this_thread::sleep(boost::posix_time::millisec(2));
	}

	static pFunc mTestFunction;
	Audio::Main mAudioMain;
	EventLoop* mEventLoop;
};

}

#endif