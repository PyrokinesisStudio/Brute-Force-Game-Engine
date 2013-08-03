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


#include <Base/Logger.h>
#include <Base/ShowException.h>

#include <tests/AudioTest/TestLoader.h>
#include <tests/AudioTest/CpuBurnTest.h>
#include <tests/AudioTest/MusicAndSoundTest.h>
//#include <tests/AudioTest/3DSoundTest.h>
#include <tests/AudioTest/StreamTest.h>
#include <tests/AudioTest/PlaylistTest.h>
#include <tests/AudioTest/SoundEmitterTest.h>

//#include <tests/AudioTest/StreamWatchTest.h>

using namespace BFG;

pFunc AudioTestLoader::mTestFunction = pFunc(NULL);

void menu()
{
	bool running = true;

	while (running)
	{
		std::cout << "menu AudioTest\n"
                  << "1: BurnCPU Test\n"
                  << "2: Music and Sound test\n"
//		          << "3: 3DSoundTest\n"
                  << "4: Stream Test\n"
                  << "5: Playlist Test\n"
                  << "6: Sound Emitter Test\n"
		          << "0: end\n\n";

		int choice;
		std::cin >> choice;

		switch (choice)
		{
            case 1:
                burnCPU();
				break;
            case 2:
			{
                AudioTestLoader::registerTestFunction(&musicAndSoundTest);
                AudioTestLoader testLoader;
				break;
			}
            case 3:
			{
//				AudioTestLoader::registerTestFunction(&test3DSound);
//				AudioTestLoader testLoader;
				break;
			}
            case 4:
			{
				AudioTestLoader::registerTestFunction(&streamTest);
				AudioTestLoader testLoader;
				break;
			}
            case 5:
			{
				AudioTestLoader::registerTestFunction(&playlistTest);
				AudioTestLoader testLoader;
			}
            case 6:
            {
                AudioTestLoader::registerTestFunction(&soundEmitterTest);
				AudioTestLoader testLoader;
                break;
            }
			case 0:
				running = false;
				break;
			default:
				std::cout << "Incorrect input. Choose again...\n\n";
		}
	}
}

int main(int argc, const char* argv[]) try
{
	Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/AudioTest.log");
	infolog << "AudioTest";
	menu();

	return 0;
}
catch (std::exception& ex)
{
	BFG::showException(ex.what());
	errlog << ex.what();
}
catch (...)
{
	BFG::showException("Unknown exception");
	errlog << "Unknown exception.";
}
