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

#ifndef SOUND_EMITTER_TEST_H_
#define SOUND_EMITTER_TEST_H_

#include <Audio/Loader.h>
#include <Audio/SoundEmitter.h>
#include <tests/AudioTest/functions.h>

void printSoundEmitterControls()
{
	std::cout << "0: Exit\n"
              << "1: Play\n"
              << "2: Pause\n"
              << "3: SingleSound\n"
              << "4: MachineGun\n";
}

void soundEmitterTest()
{
	using namespace BFG;

    Path path;

    std::string laserSound =  path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav";

    dbglog << "CreateSoundEmitter";
    Audio::SoundEmitter soundEmitter;

    //EventFactory ef;

	bool noExit = true;
	int choice = -1;
	
	while(noExit)
	{
        printSoundEmitterControls();
		std::cin >> choice;

		switch (choice)
		{
			case 0:
				noExit = false;
				break;
			case 1:
                soundEmitter.play();
				break;
			case 2:
                soundEmitter.pause();
				break;
            case 3:
                soundEmitter.processSound(laserSound);
                break;
            case 4:
            {
                for (int i = 0; i < 30; ++i)
                {
                    soundEmitter.processSound(laserSound);
                    boost::this_thread::sleep(boost::posix_time::millisec(100));
                }
                break;
            }
			default:
				std::cout << "Default";
		}
	}
}

#endif
