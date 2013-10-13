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

#ifndef LIGHTCONTROL
#define LIGHTCONTROL

#include <BaseFeature.h>

#include <map>

#include <View/Light.h>

#include <SharedData.h>


namespace Tool
{

class LightControl : public BaseFeature
{
public:
	LightControl(boost::shared_ptr<SharedData> data) :
	BaseFeature("Light", true),
	mData(data)
	{
	}

	virtual ~LightControl()
	{
		unload();
	}

	virtual void load();
	virtual void unload();

	virtual void activate();
	virtual void deactivate();

private:

	void onCloseClicked(MyGUI::Window*, const std::string& button);
	void onDiffuseChange(MyGUI::EditBox* edit);
	void onSpecularChange(MyGUI::EditBox* edit);
	void onSetAmbient(MyGUI::Widget*);
	void onCreateDirection(MyGUI::Widget*);
	void onCreatePoint(MyGUI::Widget*);
	void onCreateSpot(MyGUI::Widget*);
	void onApplyChange(MyGUI::Widget*);
	void onDeleteLight(MyGUI::Widget*);

	void onLostFocus(MyGUI::Widget* button, MyGUI::Widget*);
	void onSetFocus(MyGUI::Widget* button, MyGUI::Widget*);

	void onLightIndexChanged(MyGUI::ComboBox* sender, size_t index);

	void setTextColour(MyGUI::Widget* button, const MyGUI::Colour& colour);
	void updateLightBox();
	void fillBoxes(Ogre::Light* light);

	BFG::View::LightParameters getSpotLightValues(BFG::GameHandle handle);
	BFG::View::LightParameters getPointLightValues(BFG::GameHandle handle);
	BFG::View::LightParameters getDirectionalLightValues(BFG::GameHandle handle);
	boost::shared_ptr<SharedData> mData;

	MyGUI::EditBox* mDiffuse;
	MyGUI::EditBox* mSpecular;
	MyGUI::EditBox* mPower;
	MyGUI::EditBox* mPosition;
	MyGUI::EditBox* mDirection;
	MyGUI::EditBox* mRange;
	MyGUI::EditBox* mConstant;
	MyGUI::EditBox* mLinear;
	MyGUI::EditBox* mQuadric;
	MyGUI::EditBox* mFalloff;
	MyGUI::EditBox* mInnerRadius;
	MyGUI::EditBox* mOuterRadius;

	MyGUI::TextBox* mDif;
	MyGUI::TextBox* mSpe;
	MyGUI::TextBox* mPow;
	MyGUI::TextBox* mPos;
	MyGUI::TextBox* mDir;
	MyGUI::TextBox* mRan;
	MyGUI::TextBox* mCon;
	MyGUI::TextBox* mLin;
	MyGUI::TextBox* mQua;
	MyGUI::TextBox* mFal;
	MyGUI::TextBox* mInn;
	MyGUI::TextBox* mOut;

	MyGUI::Button* mSetAmbient;
	MyGUI::Button* mCreateDirection;
	MyGUI::Button* mCreatePoint;
	MyGUI::Button* mCreateSpot;
	MyGUI::Button* mApplyChange;
	MyGUI::Button* mDeleteLight;

	MyGUI::Widget* mColDif;
	MyGUI::Widget* mColSpec;

	MyGUI::ComboBox* mLightBox;

	typedef	std::map<BFG::GameHandle, BFG::View::Light* > LightMapT;
	LightMapT mLights;
}; // class LightControl

} // namespace Tool
#endif
