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

#ifndef COMPOSER_VIEW_STATE_H
#define COMPOSER_VIEW_STATE_H

#include <MyGUI_Widget.h>
#include <View/State.h>
#include <View/ControllerMyGuiAdapter.h>

using namespace BFG;
using namespace boost::units;

namespace Tool {

class BaseFeature;
struct SharedData;

struct ComposerViewState : public View::State
{
public:
	typedef std::vector<BaseFeature*> FeatureListT;

	ComposerViewState(GameHandle handle, Event::Lane& lane);
	~ComposerViewState();

	void createGui();

	void createViewCamera(View::CameraCreation& CC);
	void onUpdateFeatures();

	// Ogre loop
	bool frameStarted(const Ogre::FrameEvent& evt);

	bool frameRenderingQueued(const Ogre::FrameEvent& evt) { return true; }
	bool frameEnded(const Ogre::FrameEvent& evt) { return true;	}

private:
	
	virtual void pause() {}
	virtual void resume() {}

	View::ControllerMyGuiAdapter mControllerAdapter;

	FeatureListT mLoadedFeatures;
	FeatureListT mActiveFeatures;

	boost::shared_ptr<SharedData> mData;
	MyGUI::VectorWidgetPtr mContainer;

	Event::Lane& mLane;
};

} // namespace Tool

#endif