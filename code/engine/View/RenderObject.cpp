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

#include <View/RenderObject.h>

#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>

#include <View/Convert.h>
#include <View/Enums.hh>


namespace BFG {
namespace View {

RenderObject::RenderObject(Event::Lane& lane,
                           GameHandle parent, 
                           GameHandle handle,
                           const std::string& meshName,
                           const v3& position,
                           const qv4& orientation,
						   bool visible) :
mSubLane(lane.createSubLane()),
mHandle(handle),
mSceneNode(NULL),
mEntity(NULL)
{

	using namespace Ogre;
	
	Root* root = Root::getSingletonPtr();
	SceneManager* sceneMgr = root->getSceneManager(BFG_SCENEMANAGER);

	SceneNode* parentNode;

	if(parent == NULL_HANDLE)
		parentNode = sceneMgr->getRootSceneNode();
	else		
		parentNode = sceneMgr->getSceneNode(stringify(parent));	

	mSceneNode = parentNode->createChildSceneNode
	(
		stringify(handle),
		toOgre(position),
		toOgre(orientation)
	);

	mSubLane->connect(ID::VE_SET_VISIBLE, this, &RenderObject::onSetVisible, mHandle);
	mSubLane->connect(ID::VE_UPDATE_POSITION, this,	&RenderObject::updatePosition, mHandle);
	mSubLane->connect(ID::VE_UPDATE_ORIENTATION, this, &RenderObject::updateOrientation, mHandle);
	mSubLane->connect(ID::VE_ATTACH_OBJECT, this, &RenderObject::onAttachObject, mHandle);
	mSubLane->connectV(ID::VE_DETACH_OBJECT, this, &RenderObject::onDetachObject, mHandle);

	mEntity = sceneMgr->createEntity(stringify(handle), meshName);

	mSceneNode->attachObject(mEntity);

	mSceneNode->setVisible(visible);
}

RenderObject::~RenderObject()
{
	mSubLane.reset();

	Ogre::Root* root = Ogre::Root::getSingletonPtr();
	Ogre::SceneManager* sceneMgr = root->getSceneManager(BFG_SCENEMANAGER);
	
	sceneMgr->destroySceneNode(mSceneNode);
	sceneMgr->destroyEntity(mEntity);
}

void RenderObject::updatePosition(const v3& position)
{
	mSceneNode->setVisible(true);
	mSceneNode->setPosition(toOgre(position));
}

void RenderObject::updateOrientation(const qv4& orientation)
{
	mSceneNode->setVisible(true);
	mSceneNode->setOrientation(toOgre(orientation));
}

void RenderObject::onSetVisible(bool visibility)
{
	mSceneNode->setVisible(visibility);
	dbglog << "Received VE_SET_VISIBLE for #" << mHandle;
}

void RenderObject::attachOgreObject(Ogre::MovableObject* aObject)
{
	mSceneNode->attachObject(aObject);
}

void RenderObject::detachEntity(const std::string& name)
{
	mSceneNode->detachObject(name);
}

void RenderObject::onAttachObject(GameHandle child)
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);

	Ogre::SceneNode* childNode = sceneMgr->getSceneNode(stringify(child));

	Ogre::SceneNode* parentNode = childNode->getParentSceneNode();
	if (parentNode)
	{
		parentNode->removeChild(childNode);
	}
	mSceneNode->addChild(childNode);
}

void RenderObject::onDetachObject()
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);
	
	Ogre::SceneNode* parentNode = mSceneNode->getParentSceneNode();
	
	if(parentNode)
		parentNode->removeChild(stringify(mHandle));
		
	sceneMgr->getRootSceneNode()->addChild(mSceneNode);
}

void RenderObject::removeChildNode(const std::string& name)
{
	mSceneNode->removeChild(name);
}

} // namespace View
} // namespace BFG