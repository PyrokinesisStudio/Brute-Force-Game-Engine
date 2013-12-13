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

#include <MyGUI.h>

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
#include <View/View.h>

#include <Model/State.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <CameraControl.h>
#include <MeshControl.h>


using namespace BFG;
using namespace boost::units;

GameHandle gLevelerHandle = generateHandle();

struct LevelerModelState : State
{
	LevelerModelState(GameHandle handle, Event::Lane& lane) :
	State(lane),
	mLane(lane)
	{
		// Init Controller
		BFG::Controller_::ActionMapT actions;
		actions[A_QUIT] = "A_QUIT";
		actions[A_SCREENSHOT] = "A_SCREENSHOT";
		actions[A_UPDATE_FEATURES] = "A_UPDATE_FEATURES";

		BFG::Controller_::fillWithDefaultActions(actions);	
		BFG::Controller_::sendActionsToController(mLane, actions);

		Path path;
		const std::string config_path = path.Expand("Leveler.xml");
		const std::string state_name = "Leveler: He levels everything!";

		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);
		Controller_::StateInsertion si(config_path, state_name, generateHandle(), true, wa);

		mLane.emit(ID::CE_LOAD_STATE, si);
		
		mLane.connect(A_QUIT, this, &LevelerModelState::shutDown);
		mLane.connect(A_SCREENSHOT, this, &LevelerModelState::screenshot);
		mLane.connectLoop(this, &LevelerModelState::onLoop);
	}

	void shutDown(BFG::s32)
	{
		mLane.emit(BFG::ID::VE_SHUTDOWN, Event::Void());
		mLane.emit(ID::EA_FINISH, Event::Void());
	}

	void screenshot(BFG::s32)
	{
		mLane.emit(BFG::ID::VE_SCREENSHOT, Event::Void());
	}

	void onLoop(Event::TickData tickData)
	{
		onTick(tickData.timeSinceLastTick());
	}
		
	void onTick(const TimeT timeSinceLastFrame)
	{
		if (timeSinceLastFrame.value() < EPSILON_F)
			return;
	}

	Event::Lane& mLane;
};

struct LevelerViewState : public View::State
{
public:
	typedef std::vector<Tool::BaseFeature*> FeatureListT;

	LevelerViewState(GameHandle handle, Event::Lane& lane) :
	State(handle, lane),
	mControllerAdapter(handle, lane)
	{
		createGui();

		mData.reset(new SharedData);
		mData->mState = handle;
		mData->mCamera = generateHandle();

		Tool::BaseFeature* feature = new Tool::CameraControl(lane, mData);
		mLoadedFeatures.push_back(feature);
		feature->activate();

		mLoadedFeatures.push_back(new Tool::MeshControl(mData, mLane));

		mLane.connectV(A_UPDATE_FEATURES, this, &LevelerViewState::onUpdateFeatures);

		onUpdateFeatures();
	}

	~LevelerViewState()
	{
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
		mContainer = layMan->loadLayout("Leveler.layout");
		if (mContainer.empty())
			throw std::runtime_error("Leveler.layout not found!");

		Widget* box = Gui::getInstance().findWidgetT("MenuBox");
		if (!box)
			throw std::runtime_error("MenuBox not found!");

		Widget* back = Gui::getInstance().findWidgetT("Backpanel");
		if (!back)
			throw std::runtime_error("Backpanel not found");

		IntSize boxSize = box->getSize();
		IntSize size = RenderManager::getInstance().getViewSize();
		// leave 1 pixel space to the sides
		box->setSize(size.width - 2, boxSize.height);  
		back->setSize(size);
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

private:
	virtual void pause(){};
	virtual void resume(){};

	BFG::View::ControllerMyGuiAdapter mControllerAdapter;

	FeatureListT mLoadedFeatures;
	FeatureListT mActiveFeatures;

	boost::shared_ptr<SharedData> mData;

	MyGUI::VectorWidgetPtr mContainer;
};

struct Main : Base::LibraryMainBase<Event::Lane>
{
	Main() {}

	virtual void main(Event::Lane* lane)
	{
		mMain.reset(new LevelerModelState(gLevelerHandle, *lane));
	}

	boost::scoped_ptr<LevelerModelState> mMain;
};

struct ViewMain : Base::LibraryMainBase<Event::Lane>
{
	ViewMain ()
	{}

	virtual void main(Event::Lane* lane)
	{
		mViewState.reset(new LevelerViewState(gLevelerHandle, *lane));
	}

	boost::scoped_ptr<LevelerViewState> mViewState;
};

int main() try
{
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Leveler.log");

	const u32 EVENT_LOOP_FREQUENCY = 100;

	Event::Synchronizer synchronizer;

	Event::Lane viewLane(synchronizer, EVENT_LOOP_FREQUENCY, "View");
	const std::string caption = "Leveler";
	viewLane.addEntry<View::Main>(caption);
	viewLane.addEntry<ViewMain>();

	Event::Lane controllerLane(synchronizer, EVENT_LOOP_FREQUENCY, "Controller");
	controllerLane.addEntry<BFG::Controller_::Main>();

	BFG::Event::Lane gameLane(synchronizer, 100, "Game", BFG::Event::RL3);
	gameLane.addEntry<Main>();

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
