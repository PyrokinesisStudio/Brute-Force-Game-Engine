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

#ifndef CAMERACONTROL
#define CAMERACONTROL

#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include <Core/Math.h>
#include <Event/Event.h>
#include <Model/GameObject.h>
#include <Model/Environment.h>
#include <View/Convert.h>
#include <View/Owner.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <Event_fwd.h>
#include <SharedData.h>

namespace Tool
{

struct ViewParameter
{
	Ogre::Plane mPlane;
	BFG::v3 mDirection;
	BFG::qv4 mOrientation;

	ViewParameter() :
	mPlane(),
	mDirection(),
	mOrientation()
	{}

	ViewParameter(Ogre::Plane plane, BFG::v3 direction, BFG::v3 up) :
	mPlane(plane),
	mDirection(direction)
	{
		Ogre::Vector3 oUp = BFG::View::toOgre(up);
		Ogre::Vector3 oDir = BFG::View::toOgre(mDirection);
		Ogre::Vector3 oX = oUp.crossProduct(oDir);

		Ogre::Quaternion ori(oX, oUp, oDir);

		mOrientation = BFG::View::toBFG(ori);
	}
};

class CameraControl : public BaseFeature
{
public:
	typedef std::map<std::string, Ogre::SceneNode*> RacePointMap;
	
	CameraControl(BFG::Event::Lane& lane, boost::shared_ptr<SharedData> data) :
	BaseFeature("CameraControl", false),
	mSubLane(lane.createSubLane()),
	mData(data),
	mFullView(NULL),
	mCameraPosition(NULL),
	mCameraRotation(NULL),
	mCameraDistance(NULL),
	mCamerasCreated(false),
	mZoomSpeed(0.0f),
	mMoveSpeedX(0.0f),
	mMoveSpeedY(0.0f),
	mRotateSpeedX(0.0f),
	mRotateSpeedY(0.0f),
	mZoomWidget(NULL),
	mMoveWidget(NULL),
	mRotateWidget(NULL),
	mMultiview(true),
	mDeltaRot(v3::ZERO),
	mDeltaDis(0.0f),
	mCamDistance(0.0f),
	mCamOrbit(false),
	mMouseCamPitchYaw(false),
	mMouseCamRoll(false),
	mIsZooming(false),
	mRacePointName("Marker.mesh"),
	mRacePointEntity(NULL),
	mRacePointIndex(0),
	mSelectedRacePoint(NULL),
	mCreationMode(false)
	{
		mSceneMan = Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
		if (mSceneMan == NULL)
			throw std::runtime_error("SceneManager not found in CameraControl!");

		mViewParameterMap.insert
		(
			std::make_pair<std::string, ViewParameter>
			(
				"Right",
				ViewParameter
				(
					Ogre::Plane(Ogre::Vector3::UNIT_X, 0),
					BFG::v3::NEGATIVE_UNIT_X,
					BFG::v3::UNIT_Y
				)
			)
		);
		mViewParameterMap.insert
		(
			std::make_pair<std::string, ViewParameter>
			(
				"Left",
				ViewParameter
				(
					Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X, 0),
					BFG::v3::UNIT_X,
					BFG::v3::UNIT_Y
				)
			)
		);
		mViewParameterMap.insert
		(
			std::make_pair<std::string, ViewParameter>
			(
				"Top",
				ViewParameter
				(
					Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),
					BFG::v3::NEGATIVE_UNIT_Y,
					BFG::v3::UNIT_Z
				)
			)
		);
		mViewParameterMap.insert
		(
			std::make_pair<std::string, ViewParameter>
			(
				"Bottom",
				ViewParameter
				(
					Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Y, 0),
					BFG::v3::UNIT_Y,
					BFG::v3::NEGATIVE_UNIT_Z
				)
			)
		);
		mViewParameterMap.insert
		(
			std::make_pair<std::string, ViewParameter>
			(
				"Front",
				ViewParameter
				(
					Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0),
					BFG::v3::UNIT_Z,
					BFG::v3::UNIT_Y
				)
			)
		);
		mViewParameterMap.insert
		(
			std::make_pair<std::string, ViewParameter>
			(
				"Back",
				ViewParameter
				(
					Ogre::Plane(Ogre::Vector3::UNIT_Z, 0),
					BFG::v3::NEGATIVE_UNIT_Z,
					BFG::v3::UNIT_Y
				)
			)
		);

		createDefaultCamera();

		mRacePointEntity = mSceneMan->createEntity("rpPrototype", mRacePointName);
	}

	virtual ~CameraControl()
	{
		mSceneMan->destroyEntity(mRacePointEntity);
	}

	virtual void load(){mLoaded = true;}
	virtual void unload(){mLoaded = false;}

	virtual void activate()
	{
		mActive = true;
		mSubLane->emit(A_UPDATE_FEATURES, Event::Void());
	}
	virtual void deactivate()
	{
		mActive = false;
		mSubLane->emit(A_UPDATE_FEATURES, Event::Void());
	}

	void createDefaultCamera();
	void create4ViewScene();

	virtual void update(const Ogre::FrameEvent& evt);

protected:

private:
	void eventPreTextureChanges(MyGUI::Canvas* canvas);
	void removeTexture(MyGUI::Canvas* canvas);
	void requestUpdateCanvas(MyGUI::Canvas* _canvas, MyGUI::Canvas::Event _event);

	void resize();

	void onCamX(f32 x)
	{
		mDeltaRot.x = (f32)M_PI * x;
	}

	void onCamY(f32 y)
	{
		mDeltaRot.y = (f32)M_PI * y;
	}

	void onCamZ(f32 z)
	{
		mDeltaRot.z = (f32)M_PI * z;
	}

	void onReset()
	{
		mCameraRotation->setOrientation(Ogre::Quaternion::IDENTITY);
	}

	void setCanvas(MyGUI::Canvas* _value, BFG::GameHandle camHandle, Ogre::RenderTarget** target);
	void destroy(MyGUI::Canvas* _value);
	void createCam(BFG::GameHandle camHandle, Ogre::SceneNode** node, const v3& position, const qv4& orientation);

	void onMousePressed(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id);
	void onMouseReleased(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id);
	void onMouseDrag(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id);
	void onKeyReleased(MyGUI::Widget* widget, MyGUI::KeyCode key);
	void onKeyPressed(MyGUI::Widget* widget, MyGUI::KeyCode key, MyGUI::Char c);

	void intersectPosition(MyGUI::Widget* widget, int x, int y, BFG::v3& result);
	void setAspectRatio(const std::string& camName, float width, float height);
	void disableMultiview(bool disable);
	void addCanvasEvents(MyGUI::Canvas* canvas);
	void removeCanvasEvents(MyGUI::Canvas* canvas);

	const std::string checkUserString(const MyGUI::Widget* widget,
	                                  const std::string& key,
	                                  bool throwException = true) const;
	void createRacePoint(BFG::v3& position);
	void selectRacePoint(MyGUI::Widget* widget, int x, int y);
	void setSelectedRacePoint(const std::string& name);

	BFG::Event::SubLanePtr mSubLane;
	
	boost::shared_ptr<SharedData> mData;

	MyGUI::Canvas* mFullView;

	MyGUI::Canvas* mViews[4];
	Ogre::SceneNode* mViewNodes[4];
	Ogre::RenderTarget* mViewTargets[4];

	MyGUI::IntSize mFullSize;

	Ogre::SceneManager* mSceneMan;
	Ogre::SceneNode* mCameraPosition;
	Ogre::SceneNode* mCameraRotation;
	Ogre::SceneNode* mCameraDistance;

	bool mCamerasCreated;
	MyGUI::IntPoint mDragStart;
	MyGUI::IntPoint mZoomStart;
	MyGUI::IntPoint mMoveStart;
	MyGUI::IntPoint mRotateStart;
	f32 mZoomSpeed;
	f32 mMoveSpeedX;
	f32 mMoveSpeedY;
	f32 mRotateSpeedX;
	f32 mRotateSpeedY;
	MyGUI::Widget* mZoomWidget;
	MyGUI::Widget* mMoveWidget;
	MyGUI::Widget* mRotateWidget;

	bool mMultiview;

	std::map<std::string, ViewParameter> mViewParameterMap;

	v3 mDeltaRot;
	f32 mDeltaDis;
	f32 mCamDistance;
	bool mCamOrbit;
	bool mMouseCamPitchYaw;
	bool mMouseCamRoll;
	bool mIsZooming;

	const std::string mRacePointName;
	Ogre::Entity* mRacePointEntity;
	RacePointMap mRacePoints;
	u32 mRacePointIndex;
	Ogre::SceneNode* mSelectedRacePoint;
	bool mCreationMode;

}; // class CameraControl

} // namespace Tool
#endif