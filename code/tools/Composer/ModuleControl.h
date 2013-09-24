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

#ifndef MODULECONTROL
#define MODULECONTROL

#include <boost/shared_ptr.hpp>

#include <MyGUI.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <Event/Lane.h>
#include <OpenSaveDialog.h>
#include <SharedData.h>

namespace Tool
{

class ModuleControl : public BaseFeature
{
public:
	ModuleControl(BFG::Event::Lane& lane, boost::shared_ptr<SharedData> data) :
	BaseFeature("Module", true),
	mLane(lane),
	mData(data),
	mActivePreview(false)
	{
		mLane.connectV(A_UPDATE_ADAPTER, this, &ModuleControl::onUpdateAdapter);
	}

	virtual void load();
	virtual void unload();

	virtual void activate();
	virtual void deactivate();

	virtual void onUpdateAdapter();

private:
	void onCloseClicked(MyGUI::Window*, const std::string& name);

	void onAddModuleClicked(MyGUI::Widget*);
	void onCloseModuleClicked(MyGUI::Widget* sender);

	void onAddConnectionClicked(MyGUI::Widget*);
	void onCloseConnectionClicked(MyGUI::Widget* sender);

	void onLoadClicked(MyGUI::Widget*);
	void onLoadOk(MyGUI::Widget*);

	void onSaveClicked(MyGUI::Widget*);
	void onSaveOk(MyGUI::Widget*);

	void onClearClicked(MyGUI::Widget*);
	void onPreviewClicked(MyGUI::Widget*);

	void onUpdateClicked(MyGUI::Widget*);

	void destroyGameObject();

	std::string findRootModule();
	void createRoot(const std::string& name);
	void addModuleTo(const std::string& parentName);
	Module* findModule(const std::string& name);

	void addXmlModule(BFG::XmlTreeT parentNode, const Module* module, const Connection* connection);
	void addXmlConnectedModules(BFG::XmlTreeT parentNode, const std::string& rootName);

	void reAttach();
	void clearFields();
	void clearGO();

	BFG::Event::Lane& mLane;
	boost::shared_ptr<SharedData> mData;

	MyGUI::Button* mAddModule;
	MyGUI::ScrollView* mModules;

	MyGUI::Button* mAddConnection;
	MyGUI::ScrollView* mConnections;

	MyGUI::Button* mLoad;
	MyGUI::Button* mSave;
	MyGUI::Button* mClear;
	MyGUI::Button* mPreview;
	MyGUI::Button* mUpdate;

	typedef	std::map<std::string, BFG::GameHandle> ModuleMapT;
	ModuleMapT mModuleMap;

	OpenSaveDialog mDialog;

	bool mActivePreview;
}; // class ModuleControl

} // namespace Tool


#endif
