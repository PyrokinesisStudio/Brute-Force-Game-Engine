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

#include <Audio/Playlist.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <Core/strToBool.h>
#include <Audio/Main.h>


namespace BFG {
namespace Audio {

Playlist::Playlist(XmlTreeT titles, const std::string& folder):
	mRepeatAll(true),
	mState(INITIAL)
{
	load(titles, folder);
	mCurrentTrack = mTitles.begin();
	(*mCurrentTrack)->play();
	mState = PLAYING;
}

void Playlist::load(XmlTreeT tree, const std::string& folder)
{
	boost::function<void(void)> onFinishedCallback = boost::bind
	(
		&Playlist::onStreamFinishedForwarded,
		this
	);

	strToBool(tree->attribute("repeatAll"), mRepeatAll);

	XmlTreeListT titles = tree->childList("Title");

	BOOST_FOREACH(XmlTreeT t, titles)
	{
		mTitles.push_back(createAudioObject(folder + "/" + t->elementData(), mStreamLoop, onFinishedCallback));
	}
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
		(*mCurrentTrack)->play();
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
	BOOST_FOREACH(boost::shared_ptr<AudioObject> object, mTitles)
	{
		object->volume(gain);
	}
}

} // namespace Audio
} // namespace BFG
