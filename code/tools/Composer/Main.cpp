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


#include <Base/LibraryMainBase.h>
#include <Base/ShowException.h>

#include <OgreException.h>

#include <Event/Event.h>
#include <Controller/Main.h>
#include <View/Main.h>

#include <ComposerState.h>
#include <ComposerViewState.h>

GameHandle gStateHandle = BFG::generateHandle();

struct Main : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	virtual void main(BFG::Event::Lane* lane)
	{
		mMainState.reset(new ComposerState(gStateHandle, *lane));
	}

	boost::scoped_ptr<ComposerState> mMainState;
};

struct ViewMain : BFG::Base::LibraryMainBase<BFG::Event::Lane>
{
	virtual void main(BFG::Event::Lane* lane)
	{
		mViewState.reset(new Tool::ComposerViewState(gStateHandle, *lane));
	}

	boost::scoped_ptr<Tool::ComposerViewState> mViewState;
};

int main( int argc, const char* argv[] ) try
{
	const char* logFileName = "Logs/Composer.log";

#if defined(_DEBUG) || !defined(NDEBUG)
	Base::Logger::Init(Base::Logger::SL_DEBUG, logFileName);
#else
	Base::Logger::Init(Base::Logger::SL_INFORMATION, logFileName);
#endif

	BFG::Event::Synchronizer synchronizer;

	BFG::u32 eventLoopFrequency = 100;
	const std::string caption = "Composer: He composes everything!";

	BFG::Event::Lane controllerLane(synchronizer, eventLoopFrequency, "Controller");
	controllerLane.addEntry<BFG::Controller_::Main>(eventLoopFrequency);

	BFG::Event::Lane viewLane(synchronizer, eventLoopFrequency, "View");
	viewLane.addEntry<BFG::View::Main>(caption);
	viewLane.addEntry<ViewMain>();
		
	BFG::Event::Lane toolLane(synchronizer, eventLoopFrequency, "Game", BFG::Event::RL3);
	toolLane.addEntry<Main>();

	synchronizer.start();
	synchronizer.finish(true);
}
catch (Ogre::Exception& e)
{
	showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
