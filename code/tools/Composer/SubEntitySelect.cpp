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

#include <SubEntitySelect.h>

#include <OgreRoot.h>
#include <OgreEntity.h>

#include <Core/GameHandle.h>

#include <Actions.h>

namespace Tool
{

void SubEntitySelect::load()
{
	if (mLoaded)
		return;

	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("ChooseListItem.layout");

	MyGUI::Widget* w = mContainer.front();

	if (!w)
		throw std::runtime_error("ItemList not found");

	mList = w->castType<MyGUI::List>(true);

	mList->eventListSelectAccept += MyGUI::newDelegate(this, &SubEntitySelect::onEntitySelected);

	mLoaded = true;
	deactivate();
}

void SubEntitySelect::unload()
{
	if (!mLoaded)
		return;

	if (mActive)
		deactivate();

	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	layMan->unloadLayout(mContainer);

	mLoaded = false;
}

void SubEntitySelect::activate()
{
	if (mData->mActiveMesh == NULL_HANDLE)
		return;

	mMeshName = mData->mMeshNames[mData->mActiveMesh];

	if (!mLoaded)
		load();

	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(true);
	}

	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	if (sceneMgr->hasEntity(BFG::stringify(mData->mActiveMesh)))
	{
		Ogre::Entity* ent = sceneMgr->getEntity(BFG::stringify(mData->mActiveMesh));

		Ogre::MeshPtr mesh = ent->getMesh();

		const Ogre::Mesh::SubMeshNameMap subMap = mesh->getSubMeshNameMap();

		Ogre::Mesh::SubMeshNameMap::const_iterator subIt = subMap.begin();
		for (; subIt != subMap.end(); ++subIt)
		{
			mList->addItem(subIt->first);
		}
	}

	mActive = true;

	mSubLane->emit(A_UPDATE_FEATURES, BFG::Event::Void());
}

void SubEntitySelect::deactivate()
{
	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(false);
	}

	mList->removeAllItems();

	mActive = false;

	mSubLane->emit(A_UPDATE_FEATURES, BFG::Event::Void(), mData->mState);
}

void SubEntitySelect::update(const Ogre::FrameEvent& evt)
{
	if (mData->mActiveMesh == NULL_HANDLE)
		return;

	if (mMeshName != mData->mMeshNames[mData->mActiveMesh])
	{
		deactivate();
		activate();
	}
}

void SubEntitySelect::onEntitySelected(MyGUI::ListBox* list, size_t index)
{
	std::string subName = list->getItemNameAt(index);

	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	if (sceneMgr->hasEntity(stringify(mData->mActiveMesh)))
	{
		Ogre::Entity* ent = sceneMgr->getEntity(BFG::stringify(mData->mActiveMesh));

		mData->mSelectedSubEntity = ent->getSubEntity(subName);
	}

	deactivate();
}

} // namespace Tool
