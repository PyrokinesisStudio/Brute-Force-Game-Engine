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

#ifndef FUNCTIONS_H_AUDIOTEST
#define FUNCTIONS_H_AUDIOTEST

#include <iostream>
#include <boost/thread.hpp>

#include <al.h>
#include <alc.h>

#include <Core/Path.h>


void printAlError()
{
	switch (alGetError())
	{
	case AL_INVALID_NAME:
		std::cout << "AL_INVALID_NAME";
		break;
	case AL_INVALID_ENUM:
		std::cout << "AL_INVALID_ENUM";
		break;
	case AL_INVALID_VALUE:
		std::cout << "AL_INVALID_VALUE";
		break;
	case AL_INVALID_OPERATION:
		std::cout << "AL_INVALID_OPERATION";
		break;
	case AL_OUT_OF_MEMORY:
		std::cout << "AL_OUT_OF_MEMORY";
		break;
	default:
		return;
	}

	std::cout << "\n";
}

void block()
{
	std::cout << "\nInsert a number to progress.";
	int block;
	std::cin >> block;
}

void initSound(ALCcontext*& context, ALCdevice*& device)
{
	std::cout << "Audio Test suite\n\n";
	alGetError();

	std::cout << "Open device... \n";

	device = alcOpenDevice(0);

	std::cout << alGetError() << "\n";

	std::cout << "create context... \n";
	if (device)
	{
		context = alcCreateContext(device, 0);
		alcMakeContextCurrent(context);
	}

	std::cout << alGetError() << "\n";
}

std::vector<std::string> soundFileNames()
{
	using namespace BFG;

	std::vector<std::string> fileNames;
	Path path;

	fileNames.push_back(path.Get(ID::P_SOUND_MUSIC)+"6 Fleet's Arrival.ogg");
	fileNames.push_back(path.Get(ID::P_SOUND_MUSIC)+"02_Deimos - Flottenkommando.ogg");
	fileNames.push_back(path.Get(ID::P_SOUND_MUSIC)+"01_Deimos - Faint Sun.ogg");
	
	//fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"engine_light.ogg");
	//fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.wav");
    fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
	//fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
	//fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
	//fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");

	return fileNames;
}

std::vector<std::string> musicFileNames()
{
    using namespace BFG;

    std::vector<std::string> fileNames;
    Path path;

    fileNames.push_back(path.Get(ID::P_SOUND_MUSIC)+"6 Fleet's Arrival.ogg");
    fileNames.push_back(path.Get(ID::P_SOUND_MUSIC)+"02_Deimos - Flottenkommando.ogg");
    fileNames.push_back(path.Get(ID::P_SOUND_MUSIC)+"01_Deimos - Faint Sun.ogg");

    //fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"engine_light.ogg");
    //fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.wav");
    //fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
    //fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
    //fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
    //fileNames.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");

    return fileNames;
}


#endif
