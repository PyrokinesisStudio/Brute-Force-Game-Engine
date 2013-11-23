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


#include <Audio/WaveFileReader.h>

#include <stdexcept>
#include <stdlib.h>
#include <Base/Logger.h>
#include <Audio/HelperFunctions.h>

namespace BFG {
namespace Audio {

WaveFileReader::WaveFileReader(const std::string& filename) : 
mBUFFER_SIZE(44100),
mFileName(filename)
{
	mBuffer.reset(new int[mBUFFER_SIZE]);
	open();
}

WaveFileReader::~WaveFileReader()
{
	close();
}

void WaveFileReader::close()
{
	sf_close(mSoundFile);
}

void WaveFileReader::open()
{
	SF_INFO sfInfo;
	sfInfo.format = 0;

	mSoundFile = sf_open(mFileName.c_str(), SFM_READ, &sfInfo);

	if (!mSoundFile)
		throw std::logic_error("Can't open sound file: "+ mFileName +" !");

	dbglog << "channels: " << sfInfo.channels;
	dbglog << "format: " << sfInfo.format;
	dbglog << "frames: " << sfInfo.frames;
	dbglog << "samplerate: " << sfInfo.samplerate;

	if (sfInfo.channels == 1)
		mFormat = AL_FORMAT_MONO16;
	else if (sfInfo.channels == 2)
		mFormat = AL_FORMAT_STEREO16;
}

void WaveFileReader::read(ALuint bufferID)
{
	int bytesRead = static_cast<int>(sf_read_int(mSoundFile, mBuffer.get(), mBUFFER_SIZE));
	alBufferData(bufferID, mFormat, mBuffer.get(), bytesRead, mBUFFER_SIZE);
	alErrorHandler("WaveFile::read", "Error occured calling alBufferData.");
}

} // namespace Audio
} // namespace BFG
