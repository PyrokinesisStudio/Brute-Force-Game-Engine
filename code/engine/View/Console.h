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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <boost/log/sinks.hpp>
#include <OgreFrameListener.h>

#include <Base/Logger.h>      // Included to fix an operator problem with clang
#include <Event/Event.h>

namespace Ogre {
	class Rectangle2D;
}

namespace BFG {
namespace View {

//! The Class is derived from std::streambuf, so it's possible to redirect
//! the logger output.
class Console : public std::streambuf,
                Ogre::FrameListener
{
public:
	Console(Event::Lane& lane, boost::shared_ptr<Ogre::Root> root);
	~Console();
	
	//! \brief Hides or Shows the Console
	//! This method is called by View::Main since it is also the owner of
	//! the one single instance of this class. This instance does not get
	//! created and destroyed every time the Console is triggered. Instead,
	//! this function is called (for better performance).
	void toggleVisible(bool show);

private:
	//! \brief Initiates the UserInterface of the Console
	void createUI();

	//! \brief Used to connect the console Outputfield with the Logger.
	void registerSink();
	
	//! \brief Used to disconnect the console Outputfield with the Logger.
	void unregisterSink();
	
	//! \brief Override for std::streambuf
	//! This function is expected to display the first n letters in s
	int xsputn(const char* s, int n);

	//! \brief Override for std::streambuf
	//! This function is expected to display one char.
	int overflow(int);

	void onKeyPressed(s32 _code);

	//! \brief Is called, when someone presses enter in the inputline
	void onReturn();

	//! \brief Is called, when someone presses backspace in the inputline
	void onBackspace();

	//! \brief is called by pressing page_up, page_down.
	void onScroll(s32 lines);

	//! \brief Is called, when a printable character has been entered in the inputline
	void onPrintable(unsigned char);
	
	bool frameStarted(const Ogre::FrameEvent&);
	bool frameEnded(const Ogre::FrameEvent&);

	void truncateLineBuffer();
	void updateDisplayBuffer();
	
	void printDisplayBuffer();

	void onOffAnimation(const Ogre::FrameEvent& evt);

	void onHistory(bool up);

	//! For current input (before enter has been pressed)
	std::string mInputBuffer;
	std::string mBackgroundBuffer;

	//! Number of lines which are diplayed. (depends on mDisplayedLines)
	std::string mDisplayBuffer;
	std::deque<std::string> mLineBuffer;
	std::deque<std::string> mCommandBuffer;
	
	std::deque<std::string>::iterator mHistoryPointer;

	bool mHasNewContent;
	bool mDuringAnimation;
	bool mIsVisible;

	Event::SubLanePtr mSubLane;
	boost::shared_ptr<Ogre::Root> mRoot;

	f32 mHeight;
	u32 mDisplayedLines;
	size_t mScrollPosition;

	const std::string PROMT;
	const size_t MAX_LINES;
	
	boost::shared_ptr<Ogre::SceneNode> mNode;
	boost::shared_ptr<Ogre::OverlayElement> mTextBox;
	boost::shared_ptr<Ogre::Rectangle2D> mRect;

	typedef boost::log::sinks::asynchronous_sink<
		boost::log::sinks::text_ostream_backend
	> text_sink;

	boost::shared_ptr<text_sink> mSink;
};

} // namespace View
} // namespace BFG

#endif
