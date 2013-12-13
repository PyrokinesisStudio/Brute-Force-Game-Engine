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

#include <TextureControl.h>

#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>

namespace Tool
{

void TextureControl::load()
{
	using namespace MyGUI;

	LayoutManager* layMan = LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Texture.layout");

	if (mContainer.empty())
		throw std::runtime_error("Texture.layout loaded incorrectly!");
		
	Window* window = mContainer.front()->castType<Window>();

	window->eventWindowButtonPressed +=
		newDelegate(this, &TextureControl::onCloseClicked);

	mTextureUnitBox = window->findWidget("itemBox")->castType<ScrollView>();

	mLoaded = true;

	deactivate();
}

void TextureControl::unload()
{
	if (!mLoaded)
		return;

	mTextureUnits.clear();

	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	layMan->unloadLayout(mContainer);
}

void TextureControl::activate()
{
	if (mLoaded)
	{
		unload();
		load();
	}

	if (mData->mActiveMesh == BFG::NULL_HANDLE)
		return;

	mActiveHandle = mData->mActiveMesh;

	if (mData->mSelectedSubEntity == NULL)
		return;

	mActiveSubEntity = mData->mSelectedSubEntity;

	mTextureUnits.clear();

	Ogre::MaterialPtr mat = mData->mSelectedSubEntity->getMaterial();

	Ogre::Technique* tech = mat->getSupportedTechnique(0);

	unsigned int height = 0;

	Ogre::Technique::PassIterator passIt = tech->getPassIterator();
	while (passIt.hasMoreElements())
	{
		Ogre::Pass* pass = passIt.getNext();

		std::stringstream passName;
		passName << "Pass: " << pass->getName() << " (" << pass->getIndex() << ")";

		MyGUI::StaticText* staticText = mTextureUnitBox->createWidget<MyGUI::TextBox>("TextBox", 2, height, 374, 16, MyGUI::Align::Default);
		staticText->setCaption(passName.str());
		staticText->setTextColour(MyGUI::Colour::White);

		height += 16;

		Ogre::Pass::TextureUnitStateIterator texIt = pass->getTextureUnitStateIterator();
		while (texIt.hasMoreElements())
		{
			Ogre::TextureUnitState* tex = texIt.getNext();
			boost::shared_ptr<TextureUnit> tu(new TextureUnit
			(
				mTextureUnitBox,
				mat->getName(),
				tex,
				mData
			));
			mTextureUnits.push_back(tu);

			height += 80;
		}
	}

	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(true);
	}

	mActive = true;
}

void TextureControl::deactivate()
{
	if (!mLoaded)
		return;

	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(false);
	}

	mActive = false;
}


void TextureControl::update(const Ogre::FrameEvent&)
{
	checkForChanges();
}

void TextureControl::onCloseClicked(MyGUI::Window*, const std::string& button)
{
	if (button == "close")
		deactivate();
}

void TextureControl::checkForChanges()
{
	if (mActiveHandle != mData->mActiveMesh || 
		mActiveSubEntity != mData->mSelectedSubEntity)
	{
		deactivate();
		activate();
	}
}

}
