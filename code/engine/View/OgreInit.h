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

#ifndef BFG_VIEWMAIN_H
#define BFG_VIEWMAIN_H

#ifdef _MSC_VER
  #pragma warning (push)
  // "class foo needs to have dll-interface to be used by clients of class bar"
  #pragma warning (disable:4251)
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/Types.h>
#include <EventSystem/Core/EventLoop.h>

#include <View/Event_fwd.h>

namespace Ogre
{
	class Root;
	class SceneManager;
}

namespace MyGUI
{
	class Gui;
	class OgrePlatform;
}

namespace BFG {
namespace View {

class State;
class Camera;
class Fps;
class Console;

//! \brief 
//! Starting class of View. This class will create a Window, initialize Ogre and MyGUI, 
//! load all Ogre Resources and create an Ogre SceneManager and the main Camera.
//! It is responsible for the in-game Console and frames-per-second debug display.
//! It also can create screenshots.
class VIEW_API OgreInit
{
public:

	//! \brief 
	//! Constructor registers VE_SHUTDOWN, VE_CONSOLE, VE_DEBUG_FPS and VE_SCREENSHOT to
	//! eventHandler. It initializes Ogre and MyGUI.
	//! \param[in] loop EventLoop
	//! \param[in] windowTitle Window caption displayed in window mode.
	OgreInit(EventLoop* loop, const std::string& windowTitle);

	//! \brief 
	//! Destructor unregisters VE_SHUTDOWN, VE_CONSOLE, VE_DEBUG_FPS and VE_SCREENSHOT.
	virtual ~OgreInit();

private:
	//! \brief Initializes Ogre using the following four methods.
	void initOgre();

	//! \brief Initializes Ogre root by loading ogre.cfg if present or calling the Config Dialog.
	void initializeRoot();

	//! \brief Loads all resources found in paths defined in resources-win.cfg/resources-linux.cfg.
	void loadResources();

	//! \brief Creates an Ogre SceneManager with the name BFG_SCENEMANAGER defined in Defs.h.
	void createSceneManager();

	//! \brief Creates the main camera. This camera is not accessible from the outside it is necessary to initialize MyGUI.
	void createMainCamera();

	//! \brief Initializes MyGUI using guiBase.xml.
	void initMyGui();

	//! \brief Handles the following View-Events:
	//! \param[in] VE can be VE_SHUTDOWN, VE_CONSOLE (bool - display console), VE_DEBUG_FPS (bool - display FPS) or VE_SCREENSHOT.
	void eventHandler(Event* VE);
	
	//! \brief Creates a screenshot and puts it in the "Screenshot" folder.
	void onScreenShot();

	//! \brief Shows/hides the Debug frames-per-second display.
	//! \param[in] enable Show display
	void onDebugFps(bool enable);

	//! \brief Shows/hides the In-game console.
	//! \param[in] enable Show console
	void onConsole(bool enable);
	
	//! \brief Handles the Loop-Event and checks if there was an error in Ogre or if the view is shut down.
	//! \param[in] iLE LoopEvent
	void loopEventHandler(LoopEvent* iLE);

	//! \brief Calls Ogres RenderOneFrame
	bool doRenderTick();

	EventLoop* mLoop;

	bool              mShutdown;
	const std::string mWindowTitle;
	
	// Leave this on top since the auto destruction order is bottom to top,
	// and mRoot must be deleted last.
	boost::shared_ptr<Ogre::Root> mRoot;
	Ogre::SceneManager* mSceneMgr;

	// for initialization only !!
	boost::scoped_ptr<Camera> mMainCamera;
	boost::scoped_ptr<Fps> mFps;
	boost::scoped_ptr<Console> mConsole;
	
	// Leave this MyGui-pointer order because gui must be deleted first.
	boost::shared_ptr<MyGUI::OgrePlatform> mPlatform;
	boost::shared_ptr<MyGUI::Gui> mGui;
};

} // namespace View
} // namespace BFG

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#endif

