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

#include <View/Owner.h>

#include <OgreRoot.h>
#include <OgreRenderWindow.h>

#include <MyGUI.h>

#include <View/Camera.h>
#include <View/CameraCreation.h>
#include <View/Light.h>
#include <View/LightParameters.h>
#include <View/ObjectCreation.h>
#include <View/RenderObject.h>
#include <View/Skybox.h>
#include <View/SkyCreation.h>


namespace BFG {
namespace View {

Owner::Owner(GameHandle stateHandle, Event::Lane& lane) :
mLane(lane),
mSubLane(lane.createSubLane())
{
	//! \note
	//! VE_DESTROY_OBJECT makes no use of stateHandle
	mSubLane->connect(ID::VE_CREATE_OBJECT, this, &Owner::createObject, stateHandle);
	mSubLane->connect(ID::VE_DESTROY_OBJECT, this, &Owner::destroyObject);
	mSubLane->connect(ID::VE_CREATE_CAMERA, this, &Owner::createCamera, stateHandle);
	mSubLane->connect(ID::VE_SET_SKY, this, &Owner::setSky, stateHandle);
	mSubLane->connect(ID::VE_CREATE_LIGHT, this, &Owner::createLight, stateHandle);
	mSubLane->connect(ID::VE_SET_AMBIENT, this, &Owner::setAmbient, stateHandle);
}

Owner::~Owner()
{
	mSubLane.reset();
}

void Owner::createObject(const std::vector<ObjectCreation>& modules)
{
	auto itModules = modules.begin();
	for (;itModules != modules.end(); ++itModules)
	{
		boost::shared_ptr<RenderObject> ro;
		ro.reset(new RenderObject
		(
			mLane,
			*itModules
		));
		mObjects[itModules->mHandle] = ro;
	}
}

void Owner::destroyObject(GameHandle handle)
{
	ObjectMapT::iterator objIt = mObjects.find(handle);
	if (objIt != mObjects.end())
	{
		mObjects.erase(objIt);
	}
}

void Owner::createCamera(const CameraCreation& CC)
{
	Ogre::Root& root = Ogre::Root::getSingleton();

	Ogre::SceneNode* node = nullptr;
	Ogre::RenderTarget* target = nullptr;

	if (CC.mNodeHandle != NULL_HANDLE)
	{
		Ogre::SceneManager* sceneMgr =
			root.getSceneManager(BFG_SCENEMANAGER);

		node = sceneMgr->getSceneNode(stringify(CC.mNodeHandle));
	}
	if (CC.mFullscreen == true)
	{
		target = root.getAutoCreatedWindow();
	}
	boost::shared_ptr<Camera> cam;
	cam.reset(new Camera
	(
		mLane,
		CC.mHandle,
		node,
		target,
		CC.mWidth,
		CC.mHeight,
		CC.mPosition,
		CC.mOrientation,
		CC.mParentHandle
	));
	mCameras.push_back(cam);
}

void Owner::setSky(const SkyCreation& SC)
{
	mSky.reset();   // fix (skybox dtor must be called explicitly) 
	mSky.reset(new Skybox(SC.mMatName.data()));
}

void Owner::createLight(const LightParameters& LC)
{
	boost::shared_ptr<Light> light;
	light.reset(new Light(LC));
	mLights[LC.mHandle] = light;
}

void Owner::setAmbient(const cv4& colour)
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr =
		root.getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->setAmbientLight(colour);
}

} // namespace View
} // namespace BFG
