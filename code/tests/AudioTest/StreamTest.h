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

#ifndef STREAM_TEST_H_
#define STREAM_TEST_H_

#include <tests/AudioTest/functions.h>
#include <Audio/Loader.h>
#include <Audio/StreamLoop.h>
#include <Audio/OpenALAudioObject.h>


//! \Hack this won't work in further development!
void printMenu(const std::vector<std::string>& fileNameList)
{
	using namespace BFG;

    std::cout << fileNameList.size() << ": Exit\n";

    for(unsigned int i = 0; i < fileNameList.size(); ++i)
        std::cout << i << ": " << fileNameList[i] << "\n";
}

void printControls()
{
	std::cout << "0: Exit\n"
              << "1: Play\n"
	          << "2: Pause\n"
	          << "3: Stop\n";
}

void streamTest()
{
	using namespace BFG;

	dbglog << "Get EventLoop";

	std::vector<std::string> fileNameList = soundFileNames();

	boost::shared_ptr<Audio::StreamLoop> streamLoop(new Audio::StreamLoop);
	boost::shared_ptr<Audio::AudioObject> audioObject;

	bool noExit = true;

	while(noExit)
	{
		printMenu(fileNameList);

		std::cout << "Choose an audio file: ";
		int choice = -1;
		std::cin >> choice;

		if(choice >= fileNameList.size())
		{
			noExit = false;
			continue;
		}

		audioObject.reset(new Audio::OpenALAudioObject(fileNameList[choice], streamLoop));
		choice = -1;

		bool isRunning = true;
		while(isRunning)
		{
			printControls();			
			std::cin >> choice;

			switch (choice)
			{
				case 0:
					isRunning = false;
					audioObject->stop();
					break;
				case 1:
					audioObject->play();
					break;
				case 2:
					audioObject->pause();
					break;
				case 3:
					audioObject->stop();
					break;
				default:
					std::cout << "Default";
			}
		}

	}
}

#endif
