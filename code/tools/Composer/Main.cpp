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
#include <Core/ShowException.h>
#include <Core/Types.h>
#include <Core/Utils.h>
#include <EventSystem/Emitter.h>
#include <Model/State.h>
#include <View/View.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <Event.h>

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

struct ComposerState : State
{
	ComposerState(GameHandle handle, EventLoop* loop) :
	State(loop)
	{}

	void ControllerEventHandler(Controller_::VipEvent* iCE)
	{
		switch(iCE->getId())
		{
			case A_QUIT:
			{
				loop()->stop();
				break;
			}
			case A_SCREENSHOT:
			{
				emit<BFG::View::Event>(BFG::ID::VE_SCREENSHOT, 0);
				break;
			}
		}
	}

	virtual void onTick(const quantity<si::time, f32> timeSinceLastFrame)
	{}
};

struct ViewComposerState : public View::State
{
public:
	typedef std::vector<Tool::BaseFeature*> FeatureListT;

	ViewComposerState(GameHandle handle, EventLoop* loop) :
	State(handle, loop),
	mControllerAdapter(handle, loop)
	{
		registerEventHandler();
		createGui();

		mData.reset(new SharedData);
		mData->mState = handle;
		mData->mCamera = generateHandle();

		Tool::BaseFeature* feature = new Tool::CameraControl(loop, mData);
		mLoadedFeatures.push_back(feature);
		feature->activate();

		mLoadedFeatures.push_back(new Tool::MeshControl(mData));

		mLoadedFeatures.push_back(new Tool::MaterialChange(mData));

		mLoadedFeatures.push_back(new Tool::SubEntitySelect(loop, mData));

		mLoadedFeatures.push_back(new Tool::SkyBoxSelect(loop, mData));

		mLoadedFeatures.push_back(new Tool::LightControl(mData));

		mLoadedFeatures.push_back(new Tool::AdapterControl(loop, mData));

		mLoadedFeatures.push_back(new Tool::ModuleControl(loop, mData));

		mLoadedFeatures.push_back(new Tool::AnimationControl(loop, mData));

		mLoadedFeatures.push_back(new Tool::TextureControl(mData));

		onUpdateFeatures();

		Ogre::SceneManager* sceneMan = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
		sceneMan->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
		sceneMan->setShadowColour(Ogre::ColourValue(0, 0, 0));
	}

	~ViewComposerState()
	{
		emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
		
		mActiveFeatures.clear();

		FeatureListT::iterator it = mLoadedFeatures.begin();
		for (; it != mLoadedFeatures.end(); ++it)
		{
			(*it)->unload();
		}
		mLoadedFeatures.clear();
		
		if (!mContainer.empty())
		{
			MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
			layMan->unloadLayout(mContainer);
		}
	}

	void createGui()
	{
		using namespace MyGUI;

  		LayoutManager* layMan = LayoutManager::getInstancePtr();
		mContainer = layMan->loadLayout("Composer.layout");

		Gui& gui = Gui::getInstance();
		Widget* box = gui.findWidgetT("MenuBox");
		if (!box)
			throw std::runtime_error("MenuBox not found!");

		IntSize boxSize = box->getSize();
		RenderManager& renderMgr = RenderManager::getInstance();
		IntSize size = renderMgr.getViewSize();
		// leave 1 pixel space to the sides
		box->setSize(size.width - 2, boxSize.height);  
	}

	void createViewCamera(View::CameraCreation& CC)
	{
		createCamera(CC);
	}

	void controllerEventHandler(Controller_::VipEvent* ve)
	{
		FeatureListT::iterator it = mActiveFeatures.begin();
		for (; it != mActiveFeatures.end(); ++it)
		{
			(*it)->eventHandler(ve);
		}
	}

	void viewEventHandler(View::Event* VE)
	{
		dbglog << VE->getId();
	}

	void toolEventHandler(Tool::Event* TE)
	{
		switch(TE->getId())
		{
		case A_UPDATE_FEATURES:
			onUpdateFeatures();
			break;
		default:
			throw std::logic_error("Unknown ToolEvent!");
			break;
		}
	}

	void onUpdateFeatures()
	{
		mActiveFeatures.clear();

		FeatureListT::iterator it = mLoadedFeatures.begin();
		for (; it != mLoadedFeatures.end(); ++it)
		{
			bool active = (*it)->isActive();
			if (active)
			{
				mActiveFeatures.push_back(*it);
			}
		}
	}

	// Ogre loop
	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		FeatureListT::iterator it = mActiveFeatures.begin();
		for (; it != mActiveFeatures.end(); ++it)
		{
			(*it)->update(evt);
		}

		return true;
	}

	bool frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool frameEnded(const Ogre::FrameEvent& evt)
	{
		return true;
	}

private:
	void registerEventHandler()
	{
	}
	void unregisterEventHandler()
	{
	}
	
	virtual void pause(){ registerEventHandler(); }
	virtual void resume(){ unregisterEventHandler(); }

	BFG::View::ControllerMyGuiAdapter mControllerAdapter;

	FeatureListT mLoadedFeatures;
	FeatureListT mActiveFeatures;

	boost::shared_ptr<SharedData> mData;
	MyGUI::VectorWidgetPtr mContainer;
};

boost::scoped_ptr<ViewComposerState> gViewComposerState;
boost::scoped_ptr<ComposerState> gComposerState;

// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	
	GameHandle siHandle = BFG::generateHandle();

	gViewComposerState.reset(new ViewComposerState(siHandle, loop));
	gComposerState.reset(new ComposerState(siHandle, loop));

	// Init Controller
	GameHandle handle = generateHandle();

	{
		BFG::Controller_::ActionMapT actions;
		actions[A_QUIT] = "A_QUIT";
		actions[A_LOADING_MESH] = "A_LOADING_MESH";
		actions[A_LOADING_MATERIAL] = "A_LOADING_MATERIAL";
		actions[A_CAMERA_AXIS_X] = "A_CAMERA_AXIS_X";
		actions[A_CAMERA_AXIS_Y] = "A_CAMERA_AXIS_Y";
		actions[A_CAMERA_AXIS_Z] = "A_CAMERA_AXIS_Z";
		actions[A_CAMERA_MOVE] = "A_CAMERA_MOVE";
		actions[A_CAMERA_RESET] = "A_CAMERA_RESET";
		actions[A_CAMERA_ORBIT] = "A_CAMERA_ORBIT";
		actions[A_SCREENSHOT] = "A_SCREENSHOT";
		actions[A_LOADING_SKY] = "A_LOADING_SKY";

		actions[A_CREATE_LIGHT] = "A_CREATE_LIGHT";
		actions[A_DESTROY_LIGHT] = "A_DESTROY_LIGHT";
		actions[A_PREV_LIGHT] = "A_PREV_LIGHT";
		actions[A_NEXT_LIGHT] = "A_NEXT_LIGHT";
		actions[A_FIRST_LIGHT] = "A_FIRST_LIGHT";
		actions[A_LAST_LIGHT] = "A_LAST_LIGHT";

		actions[A_INFO_WINDOW] = "A_INFO_WINDOW";

		actions[A_CAMERA_MOUSE_X] = "A_CAMERA_MOUSE_X";
		actions[A_CAMERA_MOUSE_Y] = "A_CAMERA_MOUSE_Y";
		actions[A_CAMERA_MOUSE_Z] = "A_CAMERA_MOUSE_Z";
		actions[A_CAMERA_MOUSE_MOVE] = "A_CAMERA_MOUSE_MOVE";

		actions[A_SUB_MESH] = "A_SUB_MESH";
		actions[A_TEX_UNIT] = "A_TEX_UNIT";
		actions[A_ANIMATION] = "A_ANIMATION";
		actions[A_ADAPTER] = "A_ADAPTER";

		actions[A_UPDATE_FEATURES] = "A_UPDATE_FEATURES";

		BFG::Controller_::fillWithDefaultActions(actions);	
		BFG::Controller_::sendActionsToController(loop, actions);

		Path path;
		const std::string config_path = path.Expand("Composer.xml");
		const std::string state_name = "Composer";
		
		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);
		
		Controller_::StateInsertion si(config_path, state_name, handle, true, wa);

		EventFactory::Create<Controller_::ControlEvent>
		(
			loop,
			ID::CE_LOAD_STATE,
			si
		);

		loop->connect(A_QUIT, gComposerState.get(), &ComposerState::ControllerEventHandler);
		loop->connect(A_SCREENSHOT, gComposerState.get(), &ComposerState::ControllerEventHandler);

		loop->connect(A_LOADING_MESH, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_LOADING_MATERIAL, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_AXIS_X, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_AXIS_Y, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_AXIS_Z, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOVE, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_RESET, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_ORBIT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_LOADING_SKY, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);

		loop->connect(A_CREATE_LIGHT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_DESTROY_LIGHT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_PREV_LIGHT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_NEXT_LIGHT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_FIRST_LIGHT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_LAST_LIGHT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);

		loop->connect(A_INFO_WINDOW, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_SUB_MESH, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_TEX_UNIT, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_ANIMATION, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_ADAPTER, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);

		loop->connect(A_CAMERA_MOUSE_X, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOUSE_Y, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOUSE_Z, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(A_CAMERA_MOUSE_MOVE, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);

		loop->connect(BFG::ID::A_MOUSE_MIDDLE_PRESSED, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(BFG::ID::A_MOUSE_LEFT_PRESSED, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);
		loop->connect(BFG::ID::A_MOUSE_RIGHT_PRESSED, gViewComposerState.get(), &ViewComposerState::controllerEventHandler);

		loop->connect(A_UPDATE_FEATURES, gViewComposerState.get(), &ViewComposerState::toolEventHandler);
	}
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
