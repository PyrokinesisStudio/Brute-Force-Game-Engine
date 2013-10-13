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

#ifndef ADAPTERCONTROL
#define ADAPTERCONTROL

#include <BaseFeature.h>

#include <MyGUI.h>

#include <Core/Path.h>
#include <Event/Event.h>

#include <OpenSaveDialog.h>
#include <SharedData.h>

namespace Tool
{

class AdapterControl : public BaseFeature
{
public:
	AdapterControl(BFG::Event::SubLanePtr sublane, boost::shared_ptr<SharedData> data) :
	BaseFeature("Adapter", true),
	mSubLane(sublane),
	mData(data)
	{
		BFG::Path p;

		mDialog.setRestrictions
		(
			"",
			false,
			".xml"
		);
	}

	virtual void load();
	virtual void unload();

	virtual void activate();
	virtual void deactivate();

private:

	void setVisible(bool visible);

	void onCloseClicked(MyGUI::Window*, const std::string& button);

	void onPositionChanged(MyGUI::Edit*);
	void onPitchChanged(MyGUI::Edit* sender);
	void onYawChanged(MyGUI::Edit* sender);
	void onRollChanged(MyGUI::Edit* sender);

	void onNewGroupClicked(MyGUI::Widget*);
	void onNewAdapterClicked(MyGUI::Widget*);
	void onPickClicked(MyGUI::Widget*);
	void onLoadClicked(MyGUI::Widget*);
	void onClearClicked(MyGUI::Widget*);
	void onSaveAsClicked(MyGUI::Widget*);
	void onAppendClicked(MyGUI::Widget*);

	void onAdapterGroupChanged(MyGUI::ComboBox* list, size_t index);
	void onAdapterSelected(MyGUI::ComboBox* sender, size_t index);

	void onPickingPanelClicked(MyGUI::Widget*);

	void onLoadOk(MyGUI::Widget*);
	void onSaveOk(MyGUI::Widget*);
	void onAppendOk(MyGUI::Widget*);

	void enableFields(bool enable);
	void refreshAdapter();
	void clearAdapterFields();
	void updateOrientation();
	void fillGroupBox();

	void showMarker
	(
		bool show,
		const Ogre::Vector3& position = Ogre::Vector3::ZERO,
		const Ogre::Quaternion& orientation = Ogre::Quaternion::IDENTITY
	);


	BFG::Event::SubLanePtr mSubLane;

	boost::shared_ptr<SharedData> mData;

	MyGUI::EditBox* mGroupName;
	MyGUI::EditBox* mPosition;
	MyGUI::EditBox* mOrientation;
	MyGUI::EditBox* mPitch;
	MyGUI::EditBox* mYaw;
	MyGUI::EditBox* mRoll;

	MyGUI::ComboBox* mGroupBox;
	MyGUI::ComboBox* mAdapterBox;

	MyGUI::Button* mNewGroup;
	MyGUI::Button* mNewAdapter;
	MyGUI::Button* mPick;
	MyGUI::Button* mLoad;
	MyGUI::Button* mClear;
	MyGUI::Button* mSaveAs;
	MyGUI::Button* mAppend;

	MyGUI::Widget* mPickingPanel;

	Ogre::SceneNode* mMarkerNode;

	OpenSaveDialog mDialog;

}; // class AdapterControl

} // namespace Tool
#endif