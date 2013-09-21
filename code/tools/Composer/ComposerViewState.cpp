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

#include <ComposerViewState.h>

//#include <OgreSceneManager.h>
#include <OgreRoot.h>

#include <CameraControl.h>
#include <MeshControl.h>
#include <MaterialChange.h>
#include <SubEntitySelect.h>
#include <SkyBoxSelect.h>
#include <LightControl.h>
#include <AdapterControl.h>
#include <ModuleControl.h>
#include <AnimationControl.h>
#include <TextureControl.h>

#include <SharedData.h>

namespace Tool
{

typedef std::vector<BaseFeature*> FeatureListT;

ComposerViewState::ComposerViewState(GameHandle handle, Event::Lane& lane) :
	State(handle, lane),
	mControllerAdapter(handle, lane),
	mLane(lane)
{
	createGui();

	mData.reset(new SharedData);
	mData->mState = handle;
	mData->mCamera = generateHandle();

	BaseFeature* feature = new CameraControl(lane.createSubLane(), mData);
	mLoadedFeatures.push_back(feature);
	feature->activate();

	mLoadedFeatures.push_back(new MeshControl(mData, lane));
	mLoadedFeatures.push_back(new MaterialChange(mData));
	mLoadedFeatures.push_back(new SubEntitySelect(lane.createSubLane(), mData));
	mLoadedFeatures.push_back(new SkyBoxSelect(lane.createSubLane(), mData));
	mLoadedFeatures.push_back(new LightControl(mData));
	mLoadedFeatures.push_back(new AdapterControl(lane.createSubLane(), mData));
	mLoadedFeatures.push_back(new ModuleControl(lane, mData));
	mLoadedFeatures.push_back(new AnimationControl(lane.createSubLane(), mData));
	mLoadedFeatures.push_back(new TextureControl(mData));

	onUpdateFeatures();

	Ogre::SceneManager* sceneMan = Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
	sceneMan->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	sceneMan->setShadowColour(Ogre::ColourValue(0, 0, 0));

	lane.connectV(A_UPDATE_FEATURES, this, &ComposerViewState::onUpdateFeatures);
}

ComposerViewState::~ComposerViewState()
{
	mLane.emit(BFG::ID::VE_SHUTDOWN, Event::Void());
		
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

void ComposerViewState::createGui()
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

void ComposerViewState::createViewCamera(View::CameraCreation& CC)
{
	createCamera(CC);
}

void ComposerViewState::onUpdateFeatures()
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
bool ComposerViewState::frameStarted(const Ogre::FrameEvent& evt)
{
	FeatureListT::iterator it = mActiveFeatures.begin();
	for (; it != mActiveFeatures.end(); ++it)
	{
		(*it)->update(evt);
	}

	return true;
}

} // namespace Tool