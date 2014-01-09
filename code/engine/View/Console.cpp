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

#include <View/Console.h>

#include <OgreFrameListener.h>
#include <Ogre.h>

#include <Base/Cpp.h>

#include <Controller/ControllerEvents.h>

#include <Core/Path.h>
#include <Core/Math.h>
#include <Core/String.h>

#include <View/Enums.hh>

namespace logging = boost::log;

namespace BFG {
namespace View {

static std::wstring atow(const std::string& str)
{
	std::wstringstream wstrm;
	wstrm << str.c_str();
	return wstrm.str();
}

Console::Console(Event::Lane& lane, boost::shared_ptr<Ogre::Root> root) :
mHasNewContent(false),
mHistoryPointer(mCommandBuffer.end()),
mIsVisible(false),
mSubLane(lane.createSubLane()),
mRoot(root),
mHeight(1.0f),
mDisplayedLines(15),
mScrollPosition(0),
PROMT("] "),
MAX_LINES(200)
{
	createUI();
	registerSink();
	mRoot->addFrameListener(this);

	mSubLane->connect(ID::A_KEY_PRESSED, this, &Console::onKeyPressed);
}

Console::~Console()
{
	mRoot->removeFrameListener(this);
	unregisterSink();
	mSink.reset();
	mTextBox.reset();
	mRect.reset();
	mNode.reset();
	mRoot.reset();

}

void Console::toggleVisible(bool show)
{
	mInputBuffer.clear();
	mHasNewContent = true;
	mIsVisible = show;
	mDuringAnimation = true;
}

void Console::createUI()
{
	mRect.reset(new Ogre::Rectangle2D(true));
	mRect->setCorners(-1.0f, 1.0f, 1.0f, 1.0f - mHeight);
	mRect->setMaterial("console/background");
	mRect->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
	mRect->setBoundingBox
	(
		Ogre::AxisAlignedBox
		(
			-100000.0 * Ogre::Vector3::UNIT_SCALE,
			100000.0 * Ogre::Vector3::UNIT_SCALE
		)
	);
	
	Ogre::SceneManager* scene = mRoot->getSceneManager(BFG_SCENEMANAGER);
	mNode.reset(scene->getRootSceneNode()->createChildSceneNode("#Console"));
	mNode->attachObject(mRect.get());

	mTextBox.reset(Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea","ConsoleText"));
	mTextBox->setMetricsMode(Ogre::GMM_RELATIVE);
	mTextBox->setPosition(0,0);
	mTextBox->setParameter("font_name","Console");
	mTextBox->setParameter("colour_top","1 1 1");
	mTextBox->setParameter("colour_bottom","1 1 1");
	mTextBox->setParameter("char_height","0.03");

	Ogre::Overlay* overlay;
	overlay = Ogre::OverlayManager::getSingleton().create("Console");   
	overlay->add2D(static_cast<Ogre::OverlayContainer*>(mTextBox.get()));
	overlay->show();
}

void Console::registerSink()
{
	// Construct the sink
	mSink = boost::make_shared<text_sink>();

	// Add a stream to write log to
	mSink->locked_backend()->add_stream(boost::make_shared<std::ostream>(this));

	// Register the sink in the logging core
	boost::log::core::get()->add_sink(mSink);
}

void Console::unregisterSink()
{
	boost::log::core::get()->remove_sink(mSink);
}

void Console::onHistory(bool up)
{
	if (mCommandBuffer.empty())
		return;
	
	mInputBuffer.clear();
	
	if (up)
	{
		if (mHistoryPointer == mCommandBuffer.end())
			mHistoryPointer = mCommandBuffer.begin();
		else if (mHistoryPointer + 1 == mCommandBuffer.end())
			mHistoryPointer = mCommandBuffer.begin();
		else
			++mHistoryPointer;
	}
	else
	{
		if (mHistoryPointer == mCommandBuffer.begin())
			mHistoryPointer = mCommandBuffer.end();
			
		--mHistoryPointer;
	}

	mInputBuffer = *mHistoryPointer;
	mHasNewContent = true;
}

void Console::onKeyPressed(s32 _code)
{
	ID::KeyboardButton code = static_cast<ID::KeyboardButton>(_code);

	dbglog << "Got key code: " << static_cast<s32>(code);

	switch (code)
	{
		case ID::KB_RETURN:
			onReturn();
			break;
		case ID::KB_BACKSPACE:
			onBackspace();
			break;
		case ID::KB_PAGEUP:
			onScroll(1);
			break;
		case ID::KB_PAGEDOWN:
			onScroll(-1);
			break;
		case ID::KB_LALT:
		case ID::KB_RALT:
		case ID::KB_LSHIFT:
		case ID::KB_RSHIFT:
		case ID::KB_LCTRL:
		case ID::KB_RCTRL:
		case ID::KB_UP:
			onHistory(true);
			break;
		case ID::KB_DOWN:
			onHistory(false);
			break;
		case ID::KB_RIGHT:
		case ID::KB_LEFT:
		case ID::KB_CAPSLOCK:
			break;
		default:
			try
			{
				if (isprint(code))
					onPrintable(static_cast<unsigned char>(code));
			}
			catch (std::exception& e)
			{
				warnlog << "Could not translate keycode Console::onKeyPressed: " << e.what();
			}
	}
}

void Console::onScroll(s32 lines)
{
	if (mLineBuffer.size() <= mDisplayedLines)
		return;

	//! Clamps between 0 and maxBufferSize - displayedLines.
	mScrollPosition = static_cast<size_t>(clamp<s32>(mScrollPosition + lines, 0, mLineBuffer.size() - mDisplayedLines));
	updateDisplayBuffer();
}

void Console::onReturn()
{
	mSubLane->emit(ID::VE_CONSOLE_COMMAND, mInputBuffer);
	mLineBuffer.push_front(mInputBuffer + "\n");
	
	auto it = std::find(mCommandBuffer.begin(), mCommandBuffer.end(), mInputBuffer);

	if (it != mCommandBuffer.end())
		mCommandBuffer.erase(it);		

	mCommandBuffer.push_front(mInputBuffer);
	mHistoryPointer = mCommandBuffer.end();
	
	mInputBuffer.clear();
	updateDisplayBuffer();
}

void Console::onBackspace()
{
	if (!mInputBuffer.empty())
	{
		mInputBuffer.resize(mInputBuffer.size() - 1);
		mHasNewContent = true;
	}
}

void Console::onPrintable(unsigned char printable)
{
	mInputBuffer += printable;
	mHasNewContent = true;
}

void Console::truncateLineBuffer()
{
	while (mLineBuffer.size() > MAX_LINES)
		mLineBuffer.pop_back();

	while (mCommandBuffer.size() > MAX_LINES)
		mCommandBuffer.pop_back();
}

void Console::updateDisplayBuffer()
{
	auto realDisplayedLines = mLineBuffer.size() > mDisplayedLines ? mDisplayedLines : mLineBuffer.size();
	
	auto begin = mLineBuffer.rend() - mScrollPosition - realDisplayedLines;
	auto end = begin + realDisplayedLines;
	
	auto r = make_range(begin, end);

	mDisplayBuffer.clear();

	for (const auto& line : r)
		mDisplayBuffer += line;

	mHasNewContent = true;
}

void Console::printDisplayBuffer()
{
	mTextBox->setCaption(Ogre::UTFString((atow(mDisplayBuffer + PROMT + mInputBuffer))));
	mHasNewContent = false;
}

void Console::onOffAnimation(const Ogre::FrameEvent& evt)
{
	if (mIsVisible && mHeight < 1)
	{
		mHeight += evt.timeSinceLastFrame*2;
		mTextBox->show();

		if (mHeight >= 1)
		{
			mHeight = 1;
		}
	}
	else if (!mIsVisible && mHeight > 0)
	{
		mHeight -= evt.timeSinceLastFrame*2;
		if (mHeight <= 0)
		{
			mHeight = 0;
			mTextBox->hide();
		}
	}
	else
	{
		mDuringAnimation = false;
	}

	mTextBox->setPosition(0.0f, (mHeight - 1.0f) * 0.5f);
	mRect->setCorners(-1.0f, 1.0f + mHeight, 1.0f, 1.0f - mHeight);
}

bool Console::frameStarted(const Ogre::FrameEvent& evt)
{
	if (mDuringAnimation)
		onOffAnimation(evt);

	if (mHasNewContent)
		printDisplayBuffer();
	
	return true;
}

bool Console::frameEnded(const Ogre::FrameEvent& /*evt*/)
{
	return true;
}

int Console::xsputn(const char * s, int n)
{
	mLineBuffer.push_front(std::string(s, n));
	updateDisplayBuffer();
	return n;
}

int Console::overflow(int c)
{
	mBackgroundBuffer += c;
	
	if (c == '\n')	
	{
		mLineBuffer.push_front(mBackgroundBuffer);
		mBackgroundBuffer.clear();
		updateDisplayBuffer();
	}

	return 0;
}

} // namespace View
} // namespace BFG
