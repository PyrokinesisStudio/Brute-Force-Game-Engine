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

#include <View/HudElement.h>

#include <Base/Logger.h>      // Included to fix an operator problem with clang
#include <iostream>
#include <boost/log/sinks.hpp>
#include <OgreFrameListener.h>
#include <EventSystem/Emitter.h>
#include <Controller/ControllerEvents_fwd.h>

namespace Ogre {
	class Rectangle2D;
}

class EventLoop;

namespace BFG {
namespace View {

//! The Class is derived from std::streambuf, so it's possible to redirect
//! the logger output.
class Console : public std::streambuf,
                Ogre::FrameListener,
                Emitter
{
public:
	Console(EventLoop*, boost::shared_ptr<Ogre::Root> root);
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

	void eventHandler(Controller_::VipEvent* event);
	
	//! \brief Is called, when someone presses enter in the inputline
	void onReturn();

	//! \brief Is called, when someone presses backspace in the inputline
	void onBackspace();

	//! \brief Is called, when a printable character has been entered in the inputline
	void onPrintable(unsigned char);
	
	bool frameStarted(const Ogre::FrameEvent&);
	bool frameEnded(const Ogre::FrameEvent&);

	bool mHasNewContent;
	bool mIsVisible;

	std::string mConsoleContent;

	EventLoop* mLoop;

	boost::shared_ptr<Ogre::Root> mRoot;
	boost::shared_ptr<Ogre::Rectangle2D> mRect;
	boost::shared_ptr<Ogre::SceneNode> mNode;
	boost::shared_ptr<Ogre::OverlayElement> mTextBox;

	f32 mHeight;
	u32 mDisplayedLines;

	std::string mInput;

	typedef boost::log::sinks::asynchronous_sink<
		boost::log::sinks::text_ostream_backend
	> text_sink;

	boost::shared_ptr<text_sink> mSink;
};

} // namespace View
} // namespace BFG

#endif
