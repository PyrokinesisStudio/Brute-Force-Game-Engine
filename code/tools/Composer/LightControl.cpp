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

#include <LightControl.h>

#include <OgreRoot.h>

#include <MyGUI_ComboBox.h>
#include <MyGUI_Button.h>

#include <Core/GameHandle.h>
#include <Core/Types.h>

#include <View/Convert.h>
#include <View/LightParameters.h>

namespace Tool
{

using BFG::f32;
using BFG::v3;
using BFG::cv4;

void LightControl::load()
{
	if (mLoaded)
		return;

	using namespace MyGUI;

	LayoutManager* layMan = LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Light.layout");

	if (mContainer.empty())
		throw std::runtime_error("Light.layout loaded incorrectly!");

	Window* window = mContainer.front()->castType<Window>();
	window->eventWindowButtonPressed +=
		newDelegate(this, &LightControl::onCloseClicked);

	mDiffuse = window->findWidget("diffuse")->castType<EditBox>();
	mDiffuse->eventEditTextChange += 
		newDelegate(this, &LightControl::onDiffuseChange);

	mSpecular = window->findWidget("specular")->castType<EditBox>();
	mSpecular->eventEditTextChange +=
		newDelegate(this, &LightControl::onSpecularChange);

	mPower = window->findWidget("power")->castType<EditBox>();
	mPosition = window->findWidget("position")->castType<EditBox>();
	mDirection = window->findWidget("direction")->castType<EditBox>();
	mRange = window->findWidget("range")->castType<EditBox>();
	mConstant = window->findWidget("constant")->castType<EditBox>();
	mLinear = window->findWidget("linear")->castType<EditBox>();
	mQuadric = window->findWidget("quadric")->castType<EditBox>();
	mFalloff = window->findWidget("falloff")->castType<EditBox>();
	mInnerRadius = window->findWidget("innerRadius")->castType<EditBox>();
	mOuterRadius = window->findWidget("outerRadius")->castType<EditBox>();

	mDif = window->findWidget("tDif")->castType<TextBox>();
	mSpe = window->findWidget("tSpe")->castType<TextBox>();
	mPow = window->findWidget("tPow")->castType<TextBox>();
	mPos = window->findWidget("tPos")->castType<TextBox>();
	mDir = window->findWidget("tDir")->castType<TextBox>();
	mRan = window->findWidget("tRan")->castType<TextBox>();
	mCon = window->findWidget("tCon")->castType<TextBox>();
	mLin = window->findWidget("tLin")->castType<TextBox>();
	mQua = window->findWidget("tQua")->castType<TextBox>();
	mFal = window->findWidget("tFal")->castType<TextBox>();
	mInn = window->findWidget("tInn")->castType<TextBox>();
	mOut = window->findWidget("tOut")->castType<TextBox>();

	mLightBox = window->findWidget("lightBox")->castType<ComboBox>();
	mLightBox->eventComboChangePosition +=
		newDelegate(this, &LightControl::onLightIndexChanged);

	mApplyChange = window->findWidget("applyChange")->castType<Button>();
	mApplyChange->eventMouseButtonClick += 
		newDelegate(this, &LightControl::onApplyChange);
	mApplyChange->setEnabled(false);

	mDeleteLight = window->findWidget("deleteLight")->castType<Button>();
	mDeleteLight->eventMouseButtonClick +=
		newDelegate(this, &LightControl::onDeleteLight);
	mDeleteLight->setEnabled(false);

	mSetAmbient = window->findWidget("setAmbient")->castType<Button>();
	mSetAmbient->eventMouseButtonClick += 
		newDelegate(this, &LightControl::onSetAmbient);
	mSetAmbient->eventMouseSetFocus +=
		newDelegate(this, &LightControl::onSetFocus);
	mSetAmbient->eventMouseLostFocus +=
		newDelegate(this, &LightControl::onLostFocus);

	mCreateDirection = window->findWidget("createDirection")->castType<Button>();
	mCreateDirection->eventMouseButtonClick += 
		newDelegate(this, &LightControl::onCreateDirection);
	mCreateDirection->eventMouseSetFocus +=
		newDelegate(this, &LightControl::onSetFocus);
	mCreateDirection->eventMouseLostFocus +=
		newDelegate(this, &LightControl::onLostFocus);

	mCreatePoint = window->findWidget("createPoint")->castType<Button>();
	mCreatePoint->eventMouseButtonClick +=
		newDelegate(this, &LightControl::onCreatePoint);
	mCreatePoint->eventMouseSetFocus +=
		newDelegate(this, &LightControl::onSetFocus);
	mCreatePoint->eventMouseLostFocus +=
		newDelegate(this, &LightControl::onLostFocus);

	mCreateSpot = window->findWidget("createSpot")->castType<Button>();
	mCreateSpot->eventMouseButtonClick +=
		newDelegate(this, &LightControl::onCreateSpot);
	mCreateSpot->eventMouseSetFocus +=
		newDelegate(this, &LightControl::onSetFocus);
	mCreateSpot->eventMouseLostFocus +=
		newDelegate(this, &LightControl::onLostFocus);

	mColDif = window->findWidget("colDif");
	mColSpec = window->findWidget("colSpec");

	mLoaded = true;
	deactivate();
}

void LightControl::unload()
{
	if (!mLoaded)
		return;

	if (mActive)
		deactivate();

	LightMapT::iterator it = mLights.begin();
	for (; it != mLights.end(); ++it)
	{
		delete it->second;
	}
	mLights.clear();
	
	mLoaded = false;
}

void LightControl::activate()
{
	if (!mLoaded)
		load();

	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(true);
	}

	mActive = true;
}

void LightControl::deactivate()
{
	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(false);
	}

	mActive = false;
}

void LightControl::onCloseClicked(MyGUI::Window*, const std::string& button)
{
	if (button == "close")
		deactivate();
}

void LightControl::onDiffuseChange(MyGUI::EditBox* edit)
{
	cv4 col = MyGUI::utility::parseValueEx3<cv4, float>(edit->getCaption());
	mColDif->setColour(MyGUI::Colour(col.r, col.g, col.b));
}

void LightControl::onSpecularChange(MyGUI::EditBox* edit)
{
	cv4 col = MyGUI::utility::parseValueEx3<cv4, float>(edit->getCaption());
	mColSpec->setColour(MyGUI::Colour(col.r, col.g, col.b));
}

void LightControl::onSetAmbient(MyGUI::Widget*)
{
	cv4 diffuse = MyGUI::utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	
	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->setAmbientLight(diffuse);
}

BFG::View::LightParameters LightControl::getDirectionalLightValues(BFG::GameHandle handle)
{
	if (handle == BFG::NULL_HANDLE)
		handle = BFG::generateHandle();

	using namespace MyGUI;

	BFG::View::LightParameters LP;

	LP.mHandle = handle;
	LP.mType = BFG::ID::LT_Directional;
	LP.mDirection = utility::parseValueEx3<v3, float>(mDirection->getCaption());
	LP.mDiffuseColor = utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	LP.mSpecularColor = utility::parseValueEx3<cv4, float>(mSpecular->getCaption());
	LP.mPower = utility::parseFloat(mPower->getCaption());

	return LP;
}

void LightControl::onCreateDirection(MyGUI::Widget*)
{
	BFG::View::LightParameters dlc = getDirectionalLightValues(BFG::NULL_HANDLE);

	mLights.insert(std::make_pair(dlc.mHandle, new BFG::View::Light(dlc)));
	updateLightBox();
}

BFG::View::LightParameters LightControl::getPointLightValues(BFG::GameHandle handle)
{
	if (handle == BFG::NULL_HANDLE)
		handle = BFG::generateHandle();

	using namespace MyGUI;

	BFG::View::LightParameters LP;

	LP.mHandle = handle;
	LP.mType = BFG::ID::LT_Point;
	LP.mPosition = utility::parseValueEx3<v3, float>(mPosition->getCaption());
	LP.mDiffuseColor = utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	LP.mSpecularColor = utility::parseValueEx3<cv4, float>(mSpecular->getCaption());
	LP.mPower = utility::parseFloat(mPower->getCaption());
	LP.mRange = utility::parseFloat(mRange->getCaption());
	LP.mConstant = utility::parseFloat(mConstant->getCaption());
	LP.mLinear = utility::parseFloat(mLinear->getCaption());
	LP.mQuadric = utility::parseFloat(mQuadric->getCaption());

	return LP;
}

void LightControl::onCreatePoint(MyGUI::Widget*)
{
	BFG::View::LightParameters plc = getPointLightValues(BFG::NULL_HANDLE);

	mLights.insert(std::make_pair(plc.mHandle, new BFG::View::Light(plc)));
	updateLightBox();
}

BFG::View::LightParameters LightControl::getSpotLightValues(BFG::GameHandle handle)
{
	if (handle == BFG::NULL_HANDLE)
		handle = BFG::generateHandle();

	using namespace MyGUI;

	BFG::View::LightParameters LP;

	LP.mHandle = handle;
	LP.mType = BFG::ID::LT_Spot;
	LP.mPosition = utility::parseValueEx3<v3, float>(mPosition->getCaption());
	LP.mDirection = utility::parseValueEx3<v3, float>(mDirection->getCaption());
	LP.mDiffuseColor = utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	LP.mSpecularColor = utility::parseValueEx3<cv4, float>(mSpecular->getCaption());
	LP.mPower = utility::parseFloat(mPower->getCaption());
	LP.mRange = utility::parseFloat(mRange->getCaption());
	LP.mConstant = utility::parseFloat(mConstant->getCaption());
	LP.mLinear = utility::parseFloat(mLinear->getCaption());
	LP.mQuadric = utility::parseFloat(mQuadric->getCaption());
	LP.mFalloff = utility::parseFloat(mFalloff->getCaption());
	LP.mInnerRadius = utility::parseFloat(mInnerRadius->getCaption());
	LP.mOuterRadius = utility::parseFloat(mOuterRadius->getCaption());
	
	return LP;
}

void LightControl::onCreateSpot(MyGUI::Widget*)
{
	BFG::View::LightParameters slc = getSpotLightValues(BFG::NULL_HANDLE);
	
	mLights.insert(std::make_pair(slc.mHandle, new BFG::View::Light(slc)));
	updateLightBox();
}

void LightControl::onLostFocus(MyGUI::Widget* button, MyGUI::Widget*)
{
	MyGUI::LanguageManager* langMan = MyGUI::LanguageManager::getInstancePtr();
	std::string tag = langMan->getTag("BFE_Text_ColourNormal");
	MyGUI::Colour col(tag);

	setTextColour(button, col);
}

void LightControl::onSetFocus(MyGUI::Widget* button, MyGUI::Widget*)
{
	MyGUI::LanguageManager* langMan = MyGUI::LanguageManager::getInstancePtr();
	std::string tag = langMan->getTag("BFE_Text_ColourFocused");
	MyGUI::Colour col(tag);

	setTextColour(button, col);
}

void LightControl::onApplyChange(MyGUI::Widget*)
{	
	size_t index = mLightBox->getIndexSelected();
	std::string handleString = mLightBox->getItemNameAt(index);

	using namespace Ogre;

	SceneManager* sceneMan = 
		Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	Light* light = sceneMan->getLight(handleString);

	Light::LightTypes lightType = light->getType();

	BFG::GameHandle handle = BFG::destringify(handleString);

	delete mLights[handle];

	switch (lightType)
	{
	case Light::LT_DIRECTIONAL:
		{
			BFG::View::LightParameters dlc = getDirectionalLightValues(handle);
			mLights[handle] = new BFG::View::Light(dlc);
		}
		break;
	case Light::LT_POINT:
		{
			BFG::View::LightParameters plc = getPointLightValues(handle);
			mLights[handle] = new BFG::View::Light(plc);
		}
		break;
	case Light::LT_SPOTLIGHT:
		{
			BFG::View::LightParameters slc = getSpotLightValues(handle);
			mLights[handle] = new BFG::View::Light(slc);
		}
		break;
	}
}

void LightControl::onLightIndexChanged(MyGUI::ComboBox* sender, size_t index)
{
	if (index != MyGUI::ITEM_NONE)
	{
		mApplyChange->setEnabled(true);
		mDeleteLight->setEnabled(true);

		std::string handleString = sender->getItemNameAt(index);

		Ogre::SceneManager* sceneMan = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
		Ogre::Light* light = sceneMan->getLight(handleString);

		fillBoxes(light);
	}
	else
	{
		mApplyChange->setEnabled(false);
		mDeleteLight->setEnabled(false);
	}

}

void LightControl::fillBoxes(Ogre::Light* light)
{
	using namespace BFG;

	cv4 diffuse = light->getDiffuseColour();
	cv4 specular = light->getSpecularColour();
	f32 power = light->getPowerScale();
	v3 position = View::toBFG(light->getPosition());
	v3 direction = View::toBFG(light->getDirection());
	f32 range = light->getAttenuationRange();
	f32 constant = light->getAttenuationConstant();
	f32 linear = light->getAttenuationLinear();
	f32 quadric = light->getAttenuationQuadric();
	f32 falloff = light->getSpotlightFalloff();
	f32 innerRadius = light->getSpotlightInnerAngle().valueDegrees();
	f32 outerRadius = light->getSpotlightOuterAngle().valueDegrees();

	std::stringstream ss;
	
	ss << diffuse.r << " " << diffuse.g << " " << diffuse.b;
	mDiffuse->setCaption(ss.str());
	ss.str("");

	ss << specular.r << " " << specular.g << " " << specular.b;
	mSpecular->setCaption(ss.str());
	ss.str("");

	using namespace MyGUI;

	mPower->setCaption(utility::toString(power));

	ss << position.x << " " << position.y << " " << position.z;
	mPosition->setCaption(ss.str());
	ss.str("");

	ss << direction.x << " " << direction.y << " " << direction.z;
	mDirection->setCaption(ss.str());
	ss.str("");

	mRange->setCaption(utility::toString(range));
	mConstant->setCaption(utility::toString(constant));
	mLinear->setCaption(utility::toString(linear));
	mQuadric->setCaption(utility::toString(quadric));
	mFalloff->setCaption(utility::toString(falloff));
	mInnerRadius->setCaption(utility::toString(innerRadius));
	mOuterRadius->setCaption(utility::toString(outerRadius));
}

void LightControl::updateLightBox()
{
	mLightBox->removeAllItems();

	LightMapT::iterator it = mLights.begin();
	for (; it != mLights.end(); ++it)
	{
		mLightBox->addItem(BFG::stringify(it->first));
	}
}

void LightControl::setTextColour(MyGUI::Widget* button, const MyGUI::Colour& colour)
{
	if (button == mSetAmbient)
	{
		mDif->setTextColour(colour);
	}
	else if (button == mCreateDirection)
	{
		mDif->setTextColour(colour);
		mSpe->setTextColour(colour);
		mPow->setTextColour(colour);
		mDir->setTextColour(colour);
	}
	else if (button == mCreatePoint)
	{
		mDif->setTextColour(colour);
		mSpe->setTextColour(colour);
		mPow->setTextColour(colour);
		mPos->setTextColour(colour);
		mRan->setTextColour(colour);
		mCon->setTextColour(colour);
		mLin->setTextColour(colour);
		mQua->setTextColour(colour);
	}
	else if (button == mCreateSpot)
	{
		mDif->setTextColour(colour);
		mSpe->setTextColour(colour);
		mPow->setTextColour(colour);
		mPos->setTextColour(colour);
		mDir->setTextColour(colour);
		mRan->setTextColour(colour);
		mCon->setTextColour(colour);
		mLin->setTextColour(colour);
		mQua->setTextColour(colour);
		mFal->setTextColour(colour);
		mInn->setTextColour(colour);
		mOut->setTextColour(colour);
	}
}

void LightControl::onDeleteLight(MyGUI::Widget*)
{
	size_t index = mLightBox->getIndexSelected();
	std::string handleString = mLightBox->getItemNameAt(index);

	BFG::GameHandle handle = BFG::destringify(handleString);

	LightMapT::iterator it = mLights.find(handle);
	if (it == mLights.end())
		return;

	delete it->second;

	mLights.erase(it);

	mLightBox->clearIndexSelected();
	onLightIndexChanged(mLightBox, MyGUI::ITEM_NONE);

	updateLightBox();
}

} // namespace Tool
