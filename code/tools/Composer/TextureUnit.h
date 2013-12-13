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

#ifndef TEXTUREUNIT
#define TEXTUREUNIT

#include <MyGUI.h>

#include <OgreTextureUnitState.h>

#include <SharedData.h>

namespace Tool
{

const std::string DEFAULT_TEX("White.png");
const std::string DEFAULT_NORM("Flat_n.png");
const std::string DEFAULT_ILL("Black.png");

class TextureUnit
{
public:
	TextureUnit(MyGUI::ScrollView* parent,
	            const std::string& material,
	            Ogre::TextureUnitState* tex,
	            boost::shared_ptr<SharedData> data) :
	mTextureUnit(tex),
	mData(data),
	mMaterial(material),
	mParent(parent)
	{
		MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
		mContainer = layMan->loadLayout("TextureUnit.layout", "", mParent);

		size_t itemCount = mParent->getChildCount();

		BFG::s32 y = 0;
		for (size_t i = 0; i < itemCount; ++i)
		{
			MyGUI::Widget* w = mParent->getChildAt(i);
			y += w->getHeight();
		}
		BFG::s32 width = mParent->getCanvasSize().width;

		MyGUI::Widget* panel = mContainer.front();

		panel->setPosition(0, y - panel->getHeight());

		mParent->setCanvasSize(width, y);

		mAlias = panel->findWidget("alias")->castType<MyGUI::TextBox>();
		mEnabled = panel->findWidget("enabled")->castType<MyGUI::Button>();
		mName = panel->findWidget("name")->castType<MyGUI::EditBox>();
		mBias = panel->findWidget("bias")->castType<MyGUI::EditBox>();

		mEnabled->eventMouseButtonClick +=
			MyGUI::newDelegate(this, &TextureUnit::onMapEnabled);
		mName->eventEditSelectAccept +=
			MyGUI::newDelegate(this, &TextureUnit::onMaterialChanged);
		mBias->eventEditSelectAccept +=
			MyGUI::newDelegate(this, &TextureUnit::onBiasChanged);
	
		mAlias->setCaption(mTextureUnit->getTextureNameAlias());
		mName->setCaption(mTextureUnit->getTextureName());
		mBias->setCaption(MyGUI::utility::toString(mTextureUnit->getTextureMipmapBias()));

		SharedData::TucPtrT tuc = findChanges();
		if (tuc)
		{
			if (tuc->mEnabled)
			{
				mEnabled->setStateSelected(false);
			}
		}
	}

	virtual ~TextureUnit()
	{
		MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
		layMan->unloadLayout(mContainer);
	}

private:

	void onBiasChanged(MyGUI::Edit* sender)
	{
		mTextureUnit->setTextureMipmapBias(MyGUI::utility::parseValue<BFG::f32>(sender->getCaption()));
	}

	void onMaterialChanged(MyGUI::Edit* sender)
	{
		mTextureUnit->setTextureName(sender->getCaption());
	}

	void onMapEnabled(MyGUI::Widget* sender)
	{
		MyGUI::Button* button = sender->castType<MyGUI::Button>();

		if(button->getStateSelected())
		{
			SharedData::TucPtrT change = findChanges(true);

			change->mTextureName = mName->getCaption();
			change->mEnabled = true;

			button->setStateSelected(false);

			std::string alias(mAlias->getCaption());
			if (alias == "TextureMap" ||
				alias == "aoMap" ||
				alias == "SpacularMap")
			{
				mName->setCaption(DEFAULT_TEX);
			}
			else if(alias == "NormalMap")
			{
				mName->setCaption(DEFAULT_NORM);
			}
			else if(alias == "IlluminationMap")
			{
				mName->setCaption(DEFAULT_ILL);
			}
			mName->setEnabled(false);
		}
		else
		{
			SharedData::TucPtrT change = findChanges();

			if (change != nullptr)
			{
				mName->setCaption(change->mTextureName);
				change->mEnabled = false;
			}
			mName->setEnabled(true);

			button->setStateSelected(true);
		}
		onMaterialChanged(mName);
	}

	SharedData::TucPtrT findChanges(bool createIfNotFound = false)
	{
		std::string alias(mAlias->getCaption());

		SharedData::ChangeMapT::iterator it = 
			mData->mTextureChanges.find(boost::make_tuple(mMaterial, alias));

		SharedData::TucPtrT tuc;

		if (it == mData->mTextureChanges.end())
		{
			if (createIfNotFound)
			{
				tuc.reset(new SharedData::TextureUnitChange());
				mData->mTextureChanges.insert
				(
					std::make_pair
					(
						boost::make_tuple(mMaterial, alias), 
						tuc
					)
				);
			}
			return tuc;
		}

		return it->second;
	}

	Ogre::TextureUnitState* mTextureUnit;

	boost::shared_ptr<SharedData> mData;

	std::string mMaterial;

	MyGUI::ScrollView* mParent;

	MyGUI::VectorWidgetPtr mContainer;
	MyGUI::TextBox* mAlias;
	MyGUI::Button* mEnabled;
	MyGUI::EditBox* mName;
	MyGUI::EditBox* mBias;
}; // class TextureUnit

} // namespace Tool
#endif
