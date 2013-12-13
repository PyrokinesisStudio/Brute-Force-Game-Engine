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

#include <Controller/ControllerEvents.h>

#include <Core/Path.h>

#include <View/Enums.hh>

namespace logging = boost::log;

namespace BFG {
namespace View {

static std::string lastlines(const std::string& s, size_t lines)
{
	std::string result;
	std::size_t totalLines = std::count(s.begin(), s.end(), '\n');

	if (totalLines <= lines)
		return s;

	std::size_t linesTillStart = totalLines - lines;
	std::size_t start = 0;
	for (std::size_t i=0; i<linesTillStart; ++i) {
		start = s.find('\n', start + 1);
	}
	result = s.substr(start + 1);
	return result;
}

static std::wstring atow(const std::string& str)
{
	std::wstringstream wstrm;
	wstrm << str.c_str();
	return wstrm.str();
}

Console::Console(Event::Lane& lane, boost::shared_ptr<Ogre::Root> root) :
mHasNewContent(false),
mIsVisible(false),
mSubLane(lane.createSubLane()),
mRoot(root),
mHeight(1.0f),
mDisplayedLines(15)
{
	createUI();
	registerSink();
	mRoot->addFrameListener(this);

	mSubLane->connect(ID::A_KEY_PRESSED, this, &Console::onKeyPressed);
}

Console::~Console()
{
	unregisterSink();
	mRoot->removeFrameListener(this);
}

void Console::toggleVisible(bool show)
{
	mInput.clear();
	mHasNewContent = true;
	mIsVisible = show;
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
	
	//! TODO: Add logging output from OGRE
	//Ogre::LogManager::getSingleton().getDefaultLog()->addListener(this);
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

void Console::onKeyPressed(s32 _code)
{
	ID::KeyboardButton code = static_cast<ID::KeyboardButton>(_code);

	dbglog << "Got key code: " << static_cast<s32>(code);

	if (code == ID::KB_RETURN)
		onReturn();
	else if (code == ID::KB_BACKSPACE)
		onBackspace();
	else if (isprint(code))
		onPrintable(static_cast<unsigned char>(code));
}

void Console::onReturn()
{
	mSubLane->emit(ID::VE_CONSOLE_COMMAND, mInput);
	mInput.clear();
	mHasNewContent = true;
}

void Console::onBackspace()
{
	if (!mInput.empty())
		mInput.resize(mInput.size() - 1);
}

void Console::onPrintable(unsigned char printable)
{
	mInput += printable;
}

bool Console::frameStarted(const Ogre::FrameEvent& evt)
{
	// Console raise/lower animation logic
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

	mTextBox->setPosition(0.0f, (mHeight - 1.0f) * 0.5f);
	mRect->setCorners(-1.0f, 1.0f + mHeight, 1.0f, 1.0f - mHeight);

	if (mHasNewContent)
	{
		// Truncate buffer in memory
		const size_t MAX_LINES = 200;
		mConsoleContent = lastlines(mConsoleContent, MAX_LINES);

		// Display buffer
		std::string all = lastlines(mConsoleContent, mDisplayedLines);
		all += "] " + mInput;
		mTextBox->setCaption(Ogre::UTFString((atow(all))));
	}
	return true;
}

bool Console::frameEnded(const Ogre::FrameEvent& /*evt*/)
{
	return true;
}

int Console::xsputn(const char * s, int n)
{
	mConsoleContent += std::string(s, n);
	mHasNewContent = true;
	return n;
}


int Console::overflow(int c)
{
	mConsoleContent += c;
	mHasNewContent = true;
	return 0;
}

} // namespace View
} // namespace BFG
