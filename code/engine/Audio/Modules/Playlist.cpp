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

#include <Audio/Modules/Playlist.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <Core/strToBool.h>
#include <Audio/Main.h>


namespace BFG {
namespace Audio {

PlaylistXml::PlaylistXml(const std::string& filename):
	mPlaylistTag("PlayList"),
	mTitleTag("Title"),
	mRepeatAllAttribute("repeatAll")
{
	mXmlFile = createXmlFileHandle(mPath.Expand(filename));
	load();
}

void PlaylistXml::load()
{
	XmlTreeT tree = mXmlFile->root()->child(mPlaylistTag);

	strToBool(tree->attribute(mRepeatAllAttribute), mRepeatAll);

	XmlTreeListT titles = tree->childList(mTitleTag);

	BOOST_FOREACH(XmlTreeT t, titles)
	{
		std::string audioFile = t->elementData();
		mTitles.push_back(mPath.Expand(audioFile));
	}
}



Playlist::Playlist(const PlaylistXml& playlistXml):
	mRepeatAll(playlistXml.mRepeatAll),
	mState(INITIAL),
	mVolume(1.0f)
{
	boost::function<void(void)> onFinishedCallback = boost::bind
	(
		&Playlist::onStreamFinishedForwarded,
		this
	);

	BOOST_FOREACH(std::string title, playlistXml.mTitles)
	{
		mTitles.push_back(createAudioObject(title, mStreamLoop, onFinishedCallback));
	}

	volume(mVolume);

	mCurrentTrack = mTitles.begin();
	(*mCurrentTrack)->play();
	mState = PLAYING;
}

void Playlist::onStreamFinishedForwarded()
{
	++mCurrentTrack;

	if (mCurrentTrack == mTitles.end())
		if (mRepeatAll)
		{
			mCurrentTrack = mTitles.begin();
			(*mCurrentTrack)->play();
		}
		else
		{
			mState = FINISHED;
		}
	else
	{
		(*mCurrentTrack)->play();
	}
}

void Playlist::play()
{
	if (mState == PLAYING)
		return;

	if (mState == FINISHED)
		mCurrentTrack = mTitles.begin();

	(*mCurrentTrack)->play();
	mState = PLAYING;
}

void Playlist::pause()
{
	(*mCurrentTrack)->pause();
	mState = PAUSE;
}

void Playlist::volume(f32 gain)
{
	mVolume = gain;

	BOOST_FOREACH(boost::shared_ptr<AudioObject> object, mTitles)
	{
		object->volume(mVolume);
	}
}

} // namespace Audio
} // namespace BFG
