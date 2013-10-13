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

#include <AnimationControl.h>

#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <Core/GameHandle.h>

#include <Actions.h>

namespace Tool
{

void AnimationControl::load()
{
	if (mLoaded)
		return;

	using namespace MyGUI;

	LayoutManager* layMan = LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Animation.layout");

	if (mContainer.empty())
		throw std::runtime_error("Animation.layout loaded incorrectly!");

	Window* window = mContainer.front()->castType<Window>();

	window->eventWindowButtonPressed +=
		newDelegate(this, &AnimationControl::onCloseClicked);

	mStates = window->findWidget("animStates")->castType<ComboBox>();
	mSlider = window->findWidget("animSlider")->castType<ScrollBar>();
	mPlay = window->findWidget("bPlay")->castType<Button>();
	mPause = window->findWidget("bPause")->castType<Button>();
	mStop = window->findWidget("bStop")->castType<Button>();

	mStates->eventComboChangePosition += 
		newDelegate(this, &AnimationControl::onAnimationSelected);
	mSlider->eventScrollChangePosition += 
		newDelegate(this, &AnimationControl::onChangeSliderPosition);
	mPlay->eventMouseButtonClick += 
		newDelegate(this, &AnimationControl::onPlayPressed);
	mPause->eventMouseButtonClick += 
		newDelegate(this, &AnimationControl::onPausePressed);
	mStop->eventMouseButtonClick += 
		newDelegate(this, &AnimationControl::onStopPressed);

	mLoaded = true;
	deactivate();
}

void AnimationControl::unload()
{
	if (!mLoaded)
		return;

	deactivate();

	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	layMan->unloadLayout(mContainer);

	mLoaded = false;
}

void AnimationControl::activate()
{
	if (mActive)
		return;

	if (!mLoaded)
		load();

	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	if (sceneMgr == NULL)
		return;

	if (mData->mActiveMesh == NULL_HANDLE)
		return;

	Ogre::Entity* ent = sceneMgr->getEntity(stringify(mData->mActiveMesh));

	if (ent == NULL)
		return;

	Ogre::AnimationStateSet* animStateSet = ent->getAllAnimationStates();

	if (!animStateSet)
	{
		errlog << "No animation found in mesh " << mData->mActiveMesh;
		return;
	}

	Ogre::AnimationStateIterator animStateIt = animStateSet->getAnimationStateIterator();

	while(animStateIt.hasMoreElements())
	{
		Ogre::AnimationState* state = animStateIt.getNext();
		mStates->addItem(state->getAnimationName());
	}

	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(true);
	}

	mActive = true;
	mSubLane->emit(A_UPDATE_FEATURES, Event::Void());
}

void AnimationControl::deactivate()
{
	if (!mLoaded)
		return;

	mStates->removeAllItems();

	mAnimationState = NULL;
	mAnimationLoop = false;

	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(false);
	}

	mActive = false;
	mSubLane->emit(A_UPDATE_FEATURES, Event::Void());
}

void AnimationControl::update(const Ogre::FrameEvent& evt)
{
	// animation
	if (!mActive)
		return;

	if (!mAnimationState)
		return;

	if (!mAnimationLoop)
		return;

	mAnimationState->addTime(evt.timeSinceLastEvent);

	f32 pos = mAnimationState->getTimePosition();
	f32 length = mAnimationState->getLength();
	size_t sliderLength = mSlider->getScrollRange();

	size_t sliderPos = (size_t)((sliderLength - 1) * (pos / length));

	mSlider->setScrollPosition(sliderPos);
}

void AnimationControl::onCloseClicked(MyGUI::Window*, const std::string& button)
{
	if (button == "close")
		deactivate();
}

void AnimationControl::onAnimationSelected(MyGUI::ComboBox* sender, size_t index)
{
	if (mAnimationState)
	{
		mAnimationState->setEnabled(false);
		mAnimationState->setTimePosition(0.0f);
	}

	std::string stateName = sender->getItemNameAt(index);

	if (stateName.empty())
		return;

	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	Ogre::Entity* ent = sceneMgr->getEntity(stringify(mData->mActiveMesh));

	mAnimationState = ent->getAnimationState(stateName);
	mAnimationState->setEnabled(true);
}

void AnimationControl::onChangeSliderPosition(MyGUI::ScrollBar* sender, size_t newPos)
{
	if (!mAnimationState)
		return;

	f32 length = mAnimationState->getLength();
	size_t sliderLength = sender->getScrollRange() - 1;

	f32 timePos = length * ((f32)newPos / (f32)sliderLength);
	mAnimationState->setTimePosition(timePos);
}

void AnimationControl::onPlayPressed(MyGUI::Widget*)
{
	if (!mAnimationState)
		return;

	mAnimationLoop = true;
}

void AnimationControl::onPausePressed(MyGUI::Widget*)
{
	if (!mAnimationState)
		return;

	mAnimationLoop = false;
}

void AnimationControl::onStopPressed(MyGUI::Widget*)
{
	if (!mAnimationState)
		return;

	mAnimationLoop = false;
	mAnimationState->setTimePosition(0.0f);
	mSlider->setScrollPosition(0);
}

}
