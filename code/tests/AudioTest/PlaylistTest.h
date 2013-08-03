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

#ifndef PLAYLIST_TEST_H_
#define PLAYLIST_TEST_H_

#include <Audio/Loader.h>
#include <Audio/Playlist.h>
#include <tests/AudioTest/functions.h>


void printPlaylistControls()
{
	std::cout << "0: Exit\n"
              << "1: Play\n"
	          << "2: Pause\n";
}

void playlistTest()
{
	using namespace BFG;

    std::vector<std::string> fileNameList = musicFileNames();

	dbglog << "CreatePlaylist";
	Audio::Playlist playlist(fileNameList, true);

	bool noExit = true;
	int choice = -1;
	
	while(noExit)
	{
		printPlaylistControls();			
		std::cin >> choice;

		switch (choice)
		{
			case 0:
				noExit = false;
				break;
			case 1:
				playlist.play();
				break;
			case 2:
				playlist.pause();
				break;
			default:
				std::cout << "Default";
		}
	}
}

#endif
