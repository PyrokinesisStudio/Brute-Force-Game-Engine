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

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>

#include <Controller/Controller.h>
#include <Core/ClockUtils.h>
#include <Core/Path.h>
#include <Base/ShowException.h>
#include <Core/Types.h>
#include <Core/GameHandle.h>
#include <Model/State.h>
#include <View/View.h>

#include <Actions.h>
#include <BaseFeature.h>

#include <AdapterControl.h>
#include <AnimationControl.h>
#include <CameraControl.h>
#include <LightControl.h>
#include <MaterialChange.h>
#include <MeshControl.h>
#include <ModuleControl.h>
#include <SkyBoxSelect.h>
#include <SubEntitySelect.h>
#include <TextureControl.h>

using namespace BFG;
using namespace boost::units;

boost::scoped_ptr<ViewComposerState> gViewComposerState;
boost::scoped_ptr<ComposerState> gComposerState;

// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	
	GameHandle siHandle = BFG::generateHandle();

	gViewComposerState.reset(new ViewComposerState(siHandle, loop));
	gComposerState.reset(new ComposerState(siHandle, loop));


	return 0;
}

int main( int argc, const char* argv[] ) try
{
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Composer.log");

	EventLoop loop(true);

	size_t controllerFrequency = 1000;

	const std::string caption = "Composer: He composes everything!";

	BFG::View::Main viewMain(caption);
	BFG::Controller_::Main controllerMain(controllerFrequency);

	loop.addEntryPoint(viewMain.entryPoint());
	loop.addEntryPoint(controllerMain.entryPoint());
	loop.addEntryPoint(new Base::CEntryPoint(SingleThreadEntryPoint));

	loop.run();
	
	gViewComposerState.reset();
	gComposerState.reset();
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
