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

#include <AdapterControl.h>

#include <limits>

#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <tinyxml.h>

#include <Core/Mesh.h>
#include <Core/Types.h>
#include <Core/Utils.h>
#include <View/Convert.h>
#include <View/LoadMesh.h>

#include <Actions.h>

namespace Tool
{

void AdapterControl::load()
{
	if (mLoaded)
		return;

	using namespace MyGUI;

	LayoutManager* layMan = LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Adapter.layout");
	
	Widget* window = mContainer.front();
	
	// store elements
	mGroupName = window->findWidget("newGroupEdit")->castType<EditBox>();
	mPosition = window->findWidget("position")->castType<EditBox>();
	mOrientation = window->findWidget("orientation")->castType<EditBox>();
	mPitch = window->findWidget("pitch")->castType<EditBox>();
	mYaw = window->findWidget("yaw")->castType<EditBox>();
	mRoll = window->findWidget("roll")->castType<EditBox>();

	mGroupBox = window->findWidget("groupSelect")->castType<ComboBox>();
	mAdapterBox = window->findWidget("adapterSelect")->castType<ComboBox>();
	
	mNewGroup = window->findWidget("newGroup")->castType<Button>();
	mNewAdapter = window->findWidget("newAdapter")->castType<Button>();
	mPick = window->findWidget("pick")->castType<Button>();
	mLoad = window->findWidget("load")->castType<Button>();
	mClear = window->findWidget("clear")->castType<Button>();
	mSaveAs = window->findWidget("saveAs")->castType<Button>();
	mAppend = window->findWidget("append")->castType<Button>();

	// assign methods
	mPosition->eventEditSelectAccept +=
		newDelegate(this, &AdapterControl::onPositionChanged);
	mPitch->eventEditSelectAccept +=
		newDelegate(this, &AdapterControl::onPitchChanged);
	mYaw->eventEditSelectAccept +=
		newDelegate(this, &AdapterControl::onYawChanged);
	mRoll->eventEditSelectAccept +=
		newDelegate(this, &AdapterControl::onRollChanged);

	mGroupBox->eventComboChangePosition += 
		newDelegate(this, &AdapterControl::onAdapterGroupChanged);
	mAdapterBox->eventComboChangePosition +=
		newDelegate(this, &AdapterControl::onAdapterSelected);

	mNewGroup->eventMouseButtonClick = 
		newDelegate(this, &AdapterControl::onNewGroupClicked);
	mNewAdapter->eventMouseButtonClick =
		newDelegate(this, &AdapterControl::onNewAdapterClicked);
	mPick->eventMouseButtonClick = 
		newDelegate(this, &AdapterControl::onPickClicked);
	mLoad->eventMouseButtonClick =
		newDelegate(this, &AdapterControl::onLoadClicked);
	mClear->eventMouseButtonClick =
		newDelegate(this, &AdapterControl::onClearClicked);
	mSaveAs->eventMouseButtonClick =
		newDelegate(this, &AdapterControl::onSaveAsClicked);
	mAppend->eventMouseButtonClick =
		newDelegate(this, &AdapterControl::onAppendClicked);

	mNewAdapter->setEnabled(false);

	IntSize screenSize = mGui->getViewSize();

	mPickingPanel = mGui->createWidgetT
	(
		"Widget",
		"PickingPanel",
		0, 
		0,
		screenSize.width, 
		screenSize.height, 
		Align::Default, 
		"Picking"
	);
	mPickingPanel->eventMouseButtonClick =
		newDelegate(this, &AdapterControl::onPickingPanelClicked);
	mPickingPanel->setVisible(false);

	// create marker
	Ogre::SceneManager* sceneMan = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
	Ogre::SceneNode* rootSceneNode = sceneMan->getRootSceneNode();
	mMarkerNode = rootSceneNode->createChildSceneNode("MarkerNode");
	Ogre::Entity* ent = sceneMan->createEntity("AdapterMarker", "Marker.mesh");
	mMarkerNode->attachObject(ent);
	mMarkerNode->setDirection(Ogre::Vector3::UNIT_Y);
	mMarkerNode->setVisible(false);

	enableFields(false);

	mLoaded = true;
	deactivate();
}

void AdapterControl::unload()
{
	if (!mLoaded)
		return;

	if (mActive)
		deactivate();

	mLoaded = false;
}

void AdapterControl::activate()
{
	setVisible(true);

	if (mPickingPanel->getVisible())
	{
		std::string groupName(mGroupName->getCaption());

		std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[groupName];

		size_t index = mAdapterBox->getIndexSelected();

		BFG::Adapter& adapter = adapterVec.at(index);
		adapter.mParentPosition = BFG::View::toBFG(mMarkerNode->getPosition());
		adapter.mParentOrientation = BFG::View::toBFG(mMarkerNode->getOrientation());

		onAdapterSelected(mAdapterBox, index);

		mPickingPanel->setVisible(false);
	}
	mActive = true;
}

void AdapterControl::deactivate()
{
	setVisible(false);
	mActive = false;
}

void AdapterControl::setVisible(bool visible)
{
	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(visible);
	}
}

void AdapterControl::onPickClicked(MyGUI::Widget*)
{
	deactivate();
	mPickingPanel->setVisible(true);
}

void AdapterControl::onPickingPanelClicked(MyGUI::Widget*)
{
	using BFG::f32;
	using BFG::u32;

	if (mData->mActiveMesh == NULL_HANDLE)
		return;

	MyGUI::InputManager* inputMan = MyGUI::InputManager::getInstancePtr();
	Ogre::SceneManager* sceneMan = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	// Get mouse position
	const MyGUI::IntPoint& mousePos = inputMan->getMousePosition();
	MyGUI::IntSize size = mGui->getViewSize();
	f32 x = (f32)mousePos.left / (f32)size.width;
	f32 y = (f32)mousePos.top / (f32)size.height;

	Ogre::Camera* cam = sceneMan->getCamera(BFG::stringify(mData->mCamera));
	Ogre::Ray mouseRay = cam->getCameraToViewportRay(x, y);

	const std::string meshName = mData->mMeshNames[mData->mActiveMesh];

	BFG::Mesh* mesh = new BFG::Mesh(BFG::View::loadMesh(meshName));

	// Test if ray intersects with polygons
	f32 distance = std::numeric_limits<float>::max();
	Ogre::Vector3 normal;
	u32 lastIndex = mesh->mIndexCount - 2;
	for (u32 i = 0; i < lastIndex; ++i)
	{
		u32 index1 = mesh->mIndices[i];
		u32 index2 = mesh->mIndices[i+1];
		u32 index3 = mesh->mIndices[i+2];

		std::pair<bool, Ogre::Real> result = 
			Ogre::Math::intersects
			(
				mouseRay,
				BFG::View::toOgre(mesh->mVertices[index1]),
				BFG::View::toOgre(mesh->mVertices[index2]),
				BFG::View::toOgre(mesh->mVertices[index3]),
				true,
				false
			);

		if (result.first)
		{
			if (result.second < distance)
			{
				distance = result.second;

				Ogre::Plane p
				(
					BFG::View::toOgre(mesh->mVertices[index1]),
					BFG::View::toOgre(mesh->mVertices[index2]),
					BFG::View::toOgre(mesh->mVertices[index3])
				);

				normal = p.normal;
			}
		}
	}
	// only select polygons in front of the camera (no hits == max float value)
	if (distance > BFG::EPSILON_F && distance < std::numeric_limits<float>::max())
	{
		using Ogre::Vector3;
		using Ogre::Quaternion;

		Vector3 pos(mouseRay.getPoint(distance));

		Quaternion rot1 = Vector3::UNIT_Y.getRotationTo
		(
			Vector3(normal.x, normal.y, 0.0f),
			Vector3::UNIT_Z
		);

		Quaternion rot2 = rot1.yAxis().getRotationTo(normal, rot1.xAxis());
		Quaternion rot = rot2 * rot1;

		showMarker(true, pos, rot);
	}
	else
	{
		showMarker(false);
	}
}

void AdapterControl::showMarker(bool show,
                                const Ogre::Vector3& position,
                                const Ogre::Quaternion& orientation)
{
	mMarkerNode->setVisible(show);
	mMarkerNode->setPosition(position);
	mMarkerNode->setOrientation(orientation);
}

void AdapterControl::onNewGroupClicked(MyGUI::Widget*)
{
	std::string name = mGroupName->getCaption();

	if (name == "")
		return;

	size_t index = mGroupBox->findItemIndexWith(name);
	if (index == MyGUI::ITEM_NONE)
	{
		mGroupBox->addItem(name);
		index = mGroupBox->findItemIndexWith(name);
		mData->mAdapters[name] = std::vector<BFG::Adapter>();
	}
	onAdapterGroupChanged(mGroupBox, index);
}

void AdapterControl::enableFields(bool enable)
{
	std::vector<MyGUI::Widget*> fields;
	fields.push_back(mPosition);
	fields.push_back(mOrientation);
	fields.push_back(mPitch);
	fields.push_back(mYaw);
	fields.push_back(mRoll);
	fields.push_back(mAdapterBox);
	fields.push_back(mPick);
	fields.push_back(mClear);

	std::vector<MyGUI::Widget*>::iterator it = fields.begin();
	for (; it != fields.end(); ++it)
	{
		(*it)->setEnabled(enable);
	}
}

void AdapterControl::onAdapterGroupChanged(MyGUI::ComboBox* list, size_t index)
{
	if (index == MyGUI::ITEM_NONE)
	{
		enableFields(false);
		return;
	}

	list->setIndexSelected(index);
	list->beginToItemSelected();

	std::string groupname(list->getItemNameAt(index));

	mGroupName->setCaption(groupname);
	mNewAdapter->setEnabled(true);

	SharedData::AdapterMapT::iterator it = mData->mAdapters.find(groupname);
	if (it != mData->mAdapters.end())
	{
		mAdapterBox->setEnabled(true);
	}
	refreshAdapter();
}

void AdapterControl::onNewAdapterClicked(MyGUI::Widget*)
{
	std::string groupName(mGroupName->getCaption());
	std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[groupName];

	size_t index = adapterVec.size();
	BFG::Adapter adapter;
	adapter.mIdentifier = index + 1;
	adapterVec.push_back(adapter);

	enableFields(true);
	refreshAdapter();

	mAdapterBox->setIndexSelected(index);
	onAdapterSelected(mAdapterBox, index);
}

void AdapterControl::refreshAdapter()
{
	std::string groupName(mGroupName->getCaption());

	std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[groupName];

	mAdapterBox->removeAllItems();
	mAdapterBox->setCaption("");

	std::vector<BFG::Adapter>::iterator it = adapterVec.begin();
	for (; it != adapterVec.end(); ++it)
	{
		BFG::Adapter& adapter = *it;
		mAdapterBox->addItem(MyGUI::utility::toString(adapter.mIdentifier));
	}
	clearAdapterFields();
}

void AdapterControl::onAdapterSelected(MyGUI::ComboBox* sender, size_t index)
{
	std::string groupName(mGroupName->getCaption());
	std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[groupName];

	BFG::Adapter& adapter = adapterVec.at(index);

	mPosition->setCaption
	(
		MyGUI::utility::toString
		(
			adapter.mParentPosition.x, " ",
			adapter.mParentPosition.y, " ",
			adapter.mParentPosition.z
		)
	);

	mOrientation->setCaption
	(
		MyGUI::utility::toString
		(
			adapter.mParentOrientation.w, " ",
			adapter.mParentOrientation.x, " ",
			adapter.mParentOrientation.y, " ", 
			adapter.mParentOrientation.z
		)
	);

	enableFields(true);

	showMarker(true, BFG::View::toOgre(adapter.mParentPosition), BFG::View::toOgre(adapter.mParentOrientation));
}

void AdapterControl::onPositionChanged(MyGUI::Edit*)
{
	std::string groupName(mGroupName->getCaption());
	std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[groupName];

	size_t index = mAdapterBox->getIndexSelected();
	BFG::Adapter& adapter = adapterVec.at(index);
	adapter.mParentPosition = 
		MyGUI::utility::parseValueEx3<BFG::v3, BFG::f32>(mPosition->getCaption());

	showMarker
	(
		true, 
		BFG::View::toOgre(adapter.mParentPosition), 
		BFG::View::toOgre(adapter.mParentOrientation)
	);

	emit<Tool::Event>(A_UPDATE_ADAPTER, 0);
}


void AdapterControl::onPitchChanged(MyGUI::Edit* sender)
{
	BFG::f32 pitchVal = MyGUI::utility::parseFloat(sender->getCaption());
	mMarkerNode->pitch(Ogre::Degree(pitchVal));

	updateOrientation();
}

void AdapterControl::onYawChanged(MyGUI::Edit* sender)
{
	BFG::f32 yawVal = MyGUI::utility::parseFloat(sender->getCaption());
	mMarkerNode->yaw(Ogre::Degree(yawVal));

	updateOrientation();
}

void AdapterControl::onRollChanged(MyGUI::Edit* sender)
{
	BFG::f32 rollVal = MyGUI::utility::parseFloat(sender->getCaption());
	mMarkerNode->roll(Ogre::Degree(rollVal));

	updateOrientation();
}

void AdapterControl::updateOrientation()
{
	std::string groupName(mGroupName->getCaption());
	std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[groupName];

	size_t index = mAdapterBox->getIndexSelected();
	BFG::Adapter& adapter = adapterVec.at(index);

	adapter.mParentOrientation = BFG::View::toBFG(mMarkerNode->getOrientation());

	mOrientation->setCaption
	(
		MyGUI::utility::toString
		(
			adapter.mParentOrientation.w, " ",
			adapter.mParentOrientation.x, " ",
			adapter.mParentOrientation.y, " ", 
			adapter.mParentOrientation.z
		)
	);
	emit<Tool::Event>(A_UPDATE_ADAPTER, 0);
}

void AdapterControl::clearAdapterFields()
{
	mPosition->setCaption("");
	mOrientation->setCaption("");
}

void AdapterControl::onLoadClicked(MyGUI::Widget*)
{
	mDialog.setDialogInfo("Load Adapter", "Load", MyGUI::newDelegate(this, &AdapterControl::onLoadOk));
	mDialog.setVisible(true);
}

void AdapterControl::onClearClicked(MyGUI::Widget*)
{
	mData->mAdapters.clear();

	std::vector<MyGUI::Widget*> fields;
	fields.push_back(mGroupName);
	fields.push_back(mGroupBox);
	fields.push_back(mAdapterBox);
	fields.push_back(mPosition);
	fields.push_back(mOrientation);
	fields.push_back(mPitch);
	fields.push_back(mYaw);
	fields.push_back(mRoll);

	std::vector<MyGUI::Widget*>::iterator it = fields.begin();
	for (; it != fields.end(); ++it)
	{
		(*it)->setProperty("Caption", "");
	}

	mGroupBox->removeAllItems();
	mAdapterBox->removeAllItems();
	onAdapterGroupChanged(mGroupBox, MyGUI::ITEM_NONE);
}

void AdapterControl::onSaveAsClicked(MyGUI::Widget*)
{
	mDialog.setDialogInfo("Save Adapter", "Save", MyGUI::newDelegate(this, &AdapterControl::onSaveOk));
	mDialog.setVisible(true);
}

void AdapterControl::onAppendClicked(MyGUI::Widget*)
{
	mDialog.setDialogInfo("Append Adapter to", "Append", MyGUI::newDelegate(this, &AdapterControl::onAppendOk));
	mDialog.setVisible(true);
}

void AdapterControl::onLoadOk(MyGUI::Widget*)
{
	std::string filename(mDialog.getFileName());
	TiXmlDocument doc(filename);

	if (!doc.LoadFile())
	{
		throw std::runtime_error("Could not load " + filename);
	}

	TiXmlElement* adapterConfigs = doc.FirstChildElement("AdapterConfigs");
	TiXmlElement* adapterConfig = adapterConfigs->FirstChildElement("AdapterConfig");
	for (; adapterConfig != NULL; adapterConfig = adapterConfig->NextSiblingElement("AdapterConfig"))
	{
		std::string name(adapterConfig->Attribute("name"));
		std::vector<BFG::Adapter> adapterVector;

		TiXmlElement* adapter = adapterConfig->FirstChildElement("Adapter");
		for (; adapter != NULL; adapter = adapter->NextSiblingElement("Adapter"))
		{
			BFG::Adapter a;
			a.mIdentifier = MyGUI::utility::parseUInt(adapter->Attribute("id"));
			BFG::stringToVector3(adapter->Attribute("position"), a.mParentPosition);
			BFG::stringToQuaternion4(adapter->Attribute("orientation"), a.mParentOrientation);
			adapterVector.push_back(a);
		}
		mData->mAdapters[name] = adapterVector;
	}
	mDialog.setVisible(false);
	fillGroupBox();
}

void AdapterControl::fillGroupBox()
{
	mGroupBox->removeAllItems();
	mGroupBox->setCaption("");
	mGroupName->setCaption("");

	SharedData::AdapterMapT::iterator it = mData->mAdapters.begin();
	for (; it != mData->mAdapters.end(); ++it)
	{
		mGroupBox->addItem(it->first);
	}
}

void AdapterControl::onSaveOk(MyGUI::Widget*)
{
	TiXmlDocument document;
	TiXmlDeclaration* declaration = new TiXmlDeclaration("1.0", "utf-8", "" );
	document.LinkEndChild(declaration);

	TiXmlElement* adapterConfigs = new TiXmlElement("AdapterConfigs");
	document.LinkEndChild(adapterConfigs);

	SharedData::AdapterMapT::iterator mapIt = mData->mAdapters.begin();
	for (; mapIt != mData->mAdapters.end(); ++mapIt)
	{
		using BFG::v3;
		using BFG::qv4;
		using BFG::Adapter;

		if (mapIt->first == "")
			continue;

		TiXmlElement* adapters = new TiXmlElement("AdapterConfig");
		adapters->SetAttribute("name", mapIt->first);
		adapterConfigs->LinkEndChild(adapters);

		std::vector<Adapter>& adapterVec = mapIt->second;
		std::vector<Adapter>::iterator it = adapterVec.begin();
		for (; it != adapterVec.end(); ++it)
		{
			v3 pos = (*it).mParentPosition;
			TiXmlElement* adapter = new TiXmlElement("Adapter");
			adapter->SetAttribute
			(
				"id",
				(*it).mIdentifier
			);

			adapter->SetAttribute
			(
				"position",
				MyGUI::utility::toString
				(
					pos.x, ", ",
					pos.y, ", ",
					pos.z
				)
			);

			qv4 ori = (*it).mParentOrientation;
			adapter->SetAttribute
			(
				"orientation",
				MyGUI::utility::toString
				(
					ori.w, ", ",
					ori.x, ", ",
					ori.y, ", ",
					ori.z
				)
			);

			adapters->LinkEndChild(adapter);
		}

	}

	document.SaveFile(mDialog.getFileName());
	mDialog.setVisible(false);
}

void AdapterControl::onAppendOk(MyGUI::Widget*)
{
	std::string filename(mDialog.getFileName());
	TiXmlDocument doc(filename);

	if (!doc.LoadFile())
	{
		throw std::runtime_error("Could not open " + filename);
	}

	TiXmlElement* adapterConfigs = doc.FirstChildElement("AdapterConfigs");

	if (!adapterConfigs)
	{
		throw std::runtime_error("This is not an adapter file!");
	}

	SharedData::AdapterMapT::iterator mapIt = mData->mAdapters.begin();
	for (; mapIt != mData->mAdapters.end(); ++mapIt)
	{
		if (mapIt->first == "")
			continue;

		bool nameFound = false;
		int lastId = 0;

		TiXmlElement* adapterConfig = adapterConfigs->FirstChildElement("AdapterConfig");
		for (; adapterConfig != NULL; adapterConfig = adapterConfig->NextSiblingElement("AdapterConfig"))
		{
			std::string name(adapterConfig->Attribute("name"));
			if (name == mapIt->first)
			{
				nameFound = true;
				TiXmlElement* adapter = adapterConfig->FirstChildElement("Adapter");
				do 
				{
					adapter->Attribute("id", &lastId);
					adapter = adapter->NextSiblingElement("Adapter");
				} while (adapter);
				break;
			}
		}

		if (!nameFound)
		{
			adapterConfig = new TiXmlElement("AdapterConfig");
			adapterConfig->SetAttribute("name", mapIt->first);
			adapterConfigs->LinkEndChild(adapterConfig);
		}

		using BFG::Adapter;

		std::vector<Adapter>& adapterVec = mapIt->second;
		std::vector<Adapter>::iterator it = adapterVec.begin();
		for (; it != adapterVec.end(); ++it)
		{
			using BFG::v3;
			using BFG::qv4;

			v3 pos = (*it).mParentPosition;
			TiXmlElement* adapter = new TiXmlElement("Adapter");
			adapter->SetAttribute
			(
				"id",
				(*it).mIdentifier + lastId
			);
			
			adapter->SetAttribute
			(
				"position",
				MyGUI::utility::toString
				(
					pos.x, ", ",
					pos.y, ", ",
					pos.z
				)
			);
			
			qv4 ori = (*it).mParentOrientation;
			adapter->SetAttribute
			(
				"orientation",
				MyGUI::utility::toString
				(
					ori.w, ", ",
					ori.x, ", ",
					ori.y, ", ",
					ori.z
				)
			);

			adapterConfig->LinkEndChild(adapter);
		}
	}
	doc.SaveFile(filename);
	mDialog.setVisible(false);
}

} // namespace Tool
