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

#ifndef CAMERACONTROL
#define CAMERACONTROL

#include <OgreSceneNode.h>

#include <Core/Math.h>
#include <Event/Event.h>
#include <Model/GameObject.h>
#include <Model/Environment.h>
#include <View/Owner.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <Event_fwd.h>
#include <SharedData.h>

namespace Tool
{

class CameraControl : public BaseFeature
{
public:
	CameraControl(BFG::Event::SubLanePtr sublane, boost::shared_ptr<SharedData> data) :
	BaseFeature("CameraControl", false),
	mSubLane(sublane),
	mData(data),
	mCameraPosition(NULL),
	mCameraRotation(NULL),
	mCameraDistance(NULL),
	mDeltaRot(v3::ZERO),
	mDeltaDis(0.0f),
	mCamDistance(0.0f),
	mCamOrbit(false),
	mMouseCamPitchYaw(false),
	mMouseCamRoll(false),
	mIsZooming(false)
	{
		createDefaultCamera();
	}

	virtual ~CameraControl() {}

	virtual void load(){mLoaded = true;}
	virtual void unload(){mLoaded = false;}

	virtual void activate()
	{
		mActive = true;
		mSubLane->emit(A_UPDATE_FEATURES, BFG::Event::Void());
	}
	
	virtual void deactivate()
	{
		mActive = false;
		mSubLane->emit(A_UPDATE_FEATURES, BFG::Event::Void());
	}

	void createDefaultCamera();

	virtual void update(const Ogre::FrameEvent& evt);

protected:

private:

	void onCamX(f32 x)
	{
		mDeltaRot.x = (f32)M_PI * x;
	}

	void onCamY(f32 y)
	{
		mDeltaRot.y = (f32)M_PI * y;
	}

	void onCamZ(f32 z)
	{
		mDeltaRot.z = (f32)M_PI * z;
	}

	void onReset()
	{
		mCameraRotation->setOrientation(Ogre::Quaternion::IDENTITY);
	}

	BFG::Event::SubLanePtr mSubLane;

	boost::shared_ptr<SharedData> mData;

	Ogre::SceneNode* mCameraPosition;
	Ogre::SceneNode* mCameraRotation;
	Ogre::SceneNode* mCameraDistance;

	v3 mDeltaRot;
	f32 mDeltaDis;
	f32 mCamDistance;
	bool mCamOrbit;
	bool mMouseCamPitchYaw;
	bool mMouseCamRoll;
	bool mIsZooming;
}; // class CameraControl

} // namespace Tool
#endif