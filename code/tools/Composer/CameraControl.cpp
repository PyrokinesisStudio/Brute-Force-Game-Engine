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

#include <CameraControl.h>

#include <OgreRoot.h>

#include <Core/GameHandle.h>
#include <View/CameraCreation.h>


namespace Tool
{

void CameraControl::createDefaultCamera()
{
	Ogre::SceneManager* sceneMgr = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	mCameraPosition = sceneMgr->getRootSceneNode()->createChildSceneNode();
	mCameraPosition->setPosition(0, 0, 0);

	mCameraRotation = mCameraPosition->createChildSceneNode();

	mCameraDistance = mCameraRotation->createChildSceneNode(BFG::stringify(mData->mCamera));

	BFG::View::CameraCreation cc
	(
		mData->mCamera,
		mData->mCamera,
		true,
		0,
		0
	);
	
	mSubLane->emit(BFG::ID::VE_CREATE_CAMERA, cc, mData->mState);

	Ogre::Vector3 pos(0.0f, 0.0f, -mCamDistance);
	mCameraDistance->setPosition(pos);
}

void CameraControl::update(const Ogre::FrameEvent& evt)
{
	if (mCamOrbit)
	{
		mCameraRotation->rotate
		(
			Ogre::Vector3::UNIT_Y,
			Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame,
			Ogre::Node::TS_WORLD
		);

		mCameraRotation->rotate
		(
			Ogre::Vector3::UNIT_X,
			Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame,
			Ogre::Node::TS_WORLD
		);
	}
	else
	{
		mCameraRotation->yaw(Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame);
		mCameraRotation->pitch(Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame);
		mCameraRotation->roll(Ogre::Radian(mDeltaRot.z) * evt.timeSinceLastFrame);
	}

	mCamDistance += mDeltaDis * evt.timeSinceLastFrame;
	
	if (mCamDistance <= 1.0f)
		mCamDistance = 1.0f;

	mCameraDistance->setPosition(0.0f, 0.0f, -mCamDistance);

	mDeltaRot = v3::ZERO;

	if (!mIsZooming)
	{
		mDeltaDis = 0.0f;
	}
}

} // namespace Tool
