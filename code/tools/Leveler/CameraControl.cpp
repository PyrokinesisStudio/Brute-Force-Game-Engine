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

#include <CameraControl.h>

#include <MyGUI_OgrePlatform.h>

#include <Core/GameHandle.h>
#include <View/CameraCreation.h>
#include <View/Event.h>

namespace Tool
{

void CameraControl::createDefaultCamera()
{
	
	// create mygui panels
	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Camera.layout");

	if (mContainer.empty())
		throw std::runtime_error("Camera.layout found empty!");

	mFullView = mContainer.front()->castType<MyGUI::Canvas>();

	MyGUI::RenderManager* renMan = MyGUI::RenderManager::getInstancePtr();

	MyGUI::IntSize size = renMan->getViewSize();
	MyGUI::IntPoint panelPos = mFullView->getPosition();

	mFullSize = mFullView->getSize();

	mFullView->setSize(size.width - panelPos.left, size.height - panelPos.top);

	mViews[0] = mFullView->findWidget("CameraOne")->castType<MyGUI::Canvas>();
	mViews[1] = mFullView->findWidget("CameraTwo")->castType<MyGUI::Canvas>();
	mViews[2] = mFullView->findWidget("CameraThree")->castType<MyGUI::Canvas>();
	mViews[3] = mFullView->findWidget("CameraFour")->castType<MyGUI::Canvas>();

	resize();

	// create ogre cameras
	mSceneMan->destroyAllCameras();

	mCameraPosition = mSceneMan->getRootSceneNode()->createChildSceneNode();
	mCameraPosition->setPosition(0, 0, 0);

	mCameraRotation = mCameraPosition->createChildSceneNode();

	mCameraDistance = mCameraRotation->createChildSceneNode(BFG::stringify(mData->mCamera));

	BFG::View::CameraCreation cc
	(
		mData->mCamera,
		mData->mCamera,
		true,
		0,
		0
	);
	
	emit<View::Event>(BFG::ID::VE_CREATE_CAMERA, cc, mData->mState);

	for (size_t i = 0; i < 4; ++i)
	{
		mData->mCameras[i] = generateHandle();
		createCam(mData->mCameras[i], &mViewNodes[i]);
		setCanvas(mViews[i], mData->mCameras[i], &mViewTargets[i]);

		BFG::v3 position;
		BFG::qv4 orientation;
		BFG::f32 startDistance = 
			MyGUI::utility::parseFloat(mViews[i]->getUserString("startDistance"));

		const std::string look(mViews[i]->getUserString("lookFrom"));
		if (look == "Free")
		{
			position = BFG::v3::NEGATIVE_UNIT_Z * startDistance;
			orientation = BFG::qv4::IDENTITY;
		}
		else
		{
			position = (-mViewParameterMap[look].mDirection) * startDistance;
			orientation = mViewParameterMap[look].mOrientation;
		}

		emit<View::Event>(BFG::ID::VE_UPDATE_POSITION, position, mData->mCameras[i]);
		emit<View::Event>(BFG::ID::VE_UPDATE_ORIENTATION, orientation, mData->mCameras[i]);

		MyGUI::Widget* title = mViews[i]->findWidget("Title");
		const std::string type(mViews[i]->getUserString("type"));
		title->setProperty("Caption", look + " (" + type + ")");
	}
}

void CameraControl::createCam(BFG::GameHandle camHandle, Ogre::SceneNode** node)
{
	(*node) = mSceneMan->getRootSceneNode()->createChildSceneNode(stringify(camHandle));

	BFG::View::CameraCreation cc
	(
		camHandle,
		camHandle,
		false,
		mFullSize.width / 2,
		mFullSize.height / 2
	);

	emit<View::Event>(BFG::ID::VE_CREATE_CAMERA, cc, mData->mState);
}

void CameraControl::setCanvas(MyGUI::Canvas* canvas, BFG::GameHandle camHandle, Ogre::RenderTarget** target)
{
	destroy(canvas);
	
	addCanvasEvents(canvas);
	canvas->createTexture(MyGUI::Canvas::TRM_PT_VIEW_ALL, MyGUI::TextureUsage::RenderTarget);
	canvas->setUserString("camHandle", stringify(camHandle));
	canvas->setUserData(target);
	canvas->updateTexture();
}

void CameraControl::addCanvasEvents(MyGUI::Canvas* canvas)
{
	canvas->eventPreTextureChanges += MyGUI::newDelegate(this, &CameraControl::eventPreTextureChanges);
	canvas->requestUpdateCanvas = MyGUI::newDelegate(this, &CameraControl::requestUpdateCanvas);
	canvas->eventMouseButtonPressed += MyGUI::newDelegate(this, &CameraControl::onMousePressed);
	canvas->eventMouseButtonReleased += MyGUI::newDelegate(this, &CameraControl::onMouseReleased);
	canvas->eventMouseDrag += MyGUI::newDelegate(this, &CameraControl::onMouseDrag);
	canvas->eventKeyButtonReleased += MyGUI::newDelegate(this, &CameraControl::onKeyReleased);
	canvas->eventKeyButtonPressed += MyGUI::newDelegate(this, &CameraControl::onKeyPressed);
}

void CameraControl::removeCanvasEvents(MyGUI::Canvas* canvas)
{
	canvas->eventPreTextureChanges -= MyGUI::newDelegate(this, &CameraControl::eventPreTextureChanges);
	canvas->requestUpdateCanvas = nullptr;
	canvas->eventMouseButtonPressed -= MyGUI::newDelegate(this, &CameraControl::onMousePressed);
	canvas->eventMouseButtonReleased -= MyGUI::newDelegate(this, &CameraControl::onMouseReleased);
	canvas->eventMouseDrag -= MyGUI::newDelegate(this, &CameraControl::onMouseDrag);
	canvas->eventKeyButtonReleased -= MyGUI::newDelegate(this, &CameraControl::onKeyReleased);
	canvas->eventKeyButtonPressed -= MyGUI::newDelegate(this, &CameraControl::onKeyPressed);
}

void CameraControl::destroy(MyGUI::Canvas* canvas)
{
	if (canvas)
	{
		removeCanvasEvents(canvas);
		canvas->destroyTexture();
		canvas->clearUserString("camHandle");
	}
}

void CameraControl::eventPreTextureChanges(MyGUI::Canvas* canvas)
{
	removeTexture(canvas);
}

void CameraControl::removeTexture(MyGUI::Canvas* canvas)
{
	Ogre::RenderTarget** target = *canvas->getUserData<Ogre::RenderTarget**>();
	if ((*target) != nullptr)
	{
		(*target)->removeAllViewports();
		(*target) = nullptr;
		if (canvas)
			Ogre::Root::getSingleton().getRenderSystem()->destroyRenderTexture(canvas->getTexture()->getName());
	}
}

void CameraControl::requestUpdateCanvas(MyGUI::Canvas* _canvas, MyGUI::Canvas::Event _event)
{
	const std::string camName(_canvas->getUserString("camHandle"));
	if (camName.empty())
		return;
	if (!mSceneMan->hasCamera(camName))
		return;
	if (!(_event.textureChanged || _event.requested || _event.widgetResized))
		return;

	Ogre::Camera* cam = mSceneMan->getCamera(camName);

	Ogre::RenderTarget** oldTarget = *_canvas->getUserData<Ogre::RenderTarget**>();

	Ogre::TexturePtr texture = static_cast<MyGUI::OgreTexture*>(_canvas->getTexture())->getOgreTexture();
	Ogre::RenderTexture* target = texture->getBuffer()->getRenderTarget();

	if (*oldTarget != target
		&& target != nullptr
		&& cam != nullptr )
	{
		(*oldTarget) = target;
		(*oldTarget)->removeAllViewports();
		Ogre::Viewport* viewport = (*oldTarget)->addViewport(cam);
		viewport->setBackgroundColour(Ogre::ColourValue::Black);
		viewport->setClearEveryFrame(true);
		viewport->setOverlaysEnabled(false);
	}
}

void CameraControl::resize()
{
	mFullSize = mFullView->getSize();

	MyGUI::IntSize panelSize(mFullSize.width / 2, mFullSize.height / 2);

	for (size_t i = 0; i < 4; ++i)
	{
		int x = (i%2) ? panelSize.width : 0;
		int y = (i>1) ? panelSize.height : 0;

		mViews[i]->setSize(panelSize);
		mViews[i]->setPosition(x, y);

		setAspectRatio(stringify(mData->mCameras[i]), (float)panelSize.width, (float)panelSize.height);
	}

	if (mFullView->isUserString("camHandle"))
	{
		setAspectRatio
		(
			mFullView->getUserString("camHandle"), 
			(float)mFullSize.width, 
			(float)mFullSize.height
		);
	}
}

void CameraControl::disableMultiview(bool disable)
{
	mViews[0]->setVisible(!disable);
	mViews[1]->setVisible(!disable);
	mViews[2]->setVisible(!disable);
	mViews[3]->setVisible(!disable);
}

void CameraControl::setAspectRatio(const std::string& camName, float width, float height)
{
	if (mSceneMan->hasCamera(camName))
	{
		Ogre::Camera* cam = mSceneMan->getCamera(camName);
		cam->setAspectRatio(width / height);
	}
}

void CameraControl::eventHandler(BFG::Controller_::VipEvent* ve)
{

}

void CameraControl::onMousePressed(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id)
{
	if (id == MyGUI::MouseButton::Right)
	{
		mDragStart = MyGUI::IntPoint(x, y);
		if (checkUserString(widget, "lookFrom") == "Free")
		{
			mMoveStart = MyGUI::IntPoint(x, y);
			mMoveWidget = widget;
			mMoveSpeedX = 0.0f;
			mMoveSpeedY = 0.0f;
		}
	}
	else if (id == MyGUI::MouseButton::Middle)
	{
		mZoomStart = MyGUI::IntPoint(x, y);
		mZoomSpeed = 0.0f;
		mZoomWidget = widget;
	}
	else if (id == MyGUI::MouseButton::Left)
	{
		if (checkUserString(widget, "lookFrom") == "Free")
		{
			mRotateStart = MyGUI::IntPoint(x, y);
			mRotateSpeedX = 0.0f;
			mRotateSpeedY = 0.0f;
			mRotateWidget = widget;
		}
		else
		{
			if (mCreationMode)
			{
				BFG::v3 rpPosition;
				intersectPosition(widget, x, y, rpPosition);

				createRacePoint(rpPosition);
			}
			else
			{
				selectRacePoint(widget, x, y);
			}
		}
	}
}

void CameraControl::createRacePoint(BFG::v3& position)
{
	std::stringstream ss;
	ss << "RacePoint" << mRacePointIndex;
	const std::string rpName(ss.str());

	Ogre::SceneNode* rpNode = mSceneMan->getRootSceneNode()->createChildSceneNode
	(
		rpName,
		BFG::View::toOgre(position)
	);

	rpNode->attachObject(mRacePointEntity->clone(rpName));
	mRacePoints.insert(std::make_pair(rpName, rpNode));
	++mRacePointIndex;
	
	setSelectedRacePoint(rpName);
}

void CameraControl::setSelectedRacePoint(const std::string& name)
{
	if (mSelectedRacePoint)
		mSelectedRacePoint->showBoundingBox(false);

	if (name.empty())
	{
		mSelectedRacePoint = NULL;
	}
	else
	{
		mSelectedRacePoint = mRacePoints[name];
		mSelectedRacePoint->showBoundingBox(true);
	}
}

const std::string CameraControl::checkUserString(const MyGUI::Widget* widget,
                                                 const std::string& key,
                                                 bool throwException) const
{
	const std::string result(widget->getUserString(key));

	if (result.empty() && throwException)
		throw std::runtime_error(key + " not found in " + widget->getName());

	return result;
}

void CameraControl::selectRacePoint(MyGUI::Widget* widget, int x, int y)
{
	const std::string look(checkUserString(widget, "lookFrom"));
	const std::string camName(checkUserString(widget, "camHandle"));

	Ogre::Camera* cam = mSceneMan->getCamera(camName);

	MyGUI::IntCoord coord = widget->getAbsoluteCoord();
	BFG::f32 mouseX = (x - coord.left) / (BFG::f32)coord.width;
	BFG::f32 mouseY = (y - coord.top) / (BFG::f32)coord.height;

	Ogre::Ray mouseRay = cam->getCameraToViewportRay(mouseX, mouseY);

	RacePointMap::iterator it = mRacePoints.begin();
	for (; it != mRacePoints.end(); ++it)
	{
		Ogre::SceneNode* node = it->second;
		Ogre::MovableObject* object = node->getAttachedObject(it->first);

		std::pair<bool, Ogre::Real> intersection = mouseRay.intersects(object->getWorldBoundingBox());

		if (intersection.first)
		{
			setSelectedRacePoint(it->first);
			return;
		}
	}
	setSelectedRacePoint("");
}

void CameraControl::intersectPosition(MyGUI::Widget* widget, int x, int y, BFG::v3& result)
{
	const std::string look(checkUserString(widget, "lookFrom"));
	const std::string camName(checkUserString(widget, "camHandle"));

	Ogre::Camera* cam = mSceneMan->getCamera(camName);

	MyGUI::IntCoord coord = widget->getAbsoluteCoord();
	BFG::f32 mouseX = (x - coord.left) / (BFG::f32)coord.width;
	BFG::f32 mouseY = (y - coord.top) / (BFG::f32)coord.height;

	Ogre::Ray mouseRay = cam->getCameraToViewportRay(mouseX, mouseY);

	Ogre::Plane& plane = mViewParameterMap[look].mPlane;
	std::pair<bool, Ogre::Real> intersection = mouseRay.intersects(plane);

	if(intersection.first)
	{
		result = BFG::View::toBFG(mouseRay.getPoint(intersection.second));
		return;
	}

	throw std::runtime_error("No intersection with plane in widget " + widget->getName());
}

void CameraControl::onMouseReleased(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id)
{
	if (id == MyGUI::MouseButton::Right)
	{
		mDragStart = MyGUI::IntPoint(0, 0);
		if (checkUserString(widget, "lookFrom") == "Free")
		{
			mMoveStart = MyGUI::IntPoint(0, 0);
			mMoveWidget = NULL;
		}
	}
	else if (id == MyGUI::MouseButton::Middle)
	{
		mZoomStart = MyGUI::IntPoint(0,0);
		mZoomSpeed = 0.0f;
		mZoomWidget = NULL;
	}
	else if (id == MyGUI::MouseButton::Left)
	{
		mRotateStart = MyGUI::IntPoint(0, 0);
		if (checkUserString(widget, "lookFrom") == "Free")
		{
			mRotateWidget = NULL;
		}
	}
}

void CameraControl::onMouseDrag(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id)
{
	if (mDragStart.left == x && mDragStart.top == y)
		return;

	const std::string look(checkUserString(widget, "lookFrom"));
	if (id == MyGUI::MouseButton::Right)
	{
		if (look == "Free")
		{
			if (mMoveStart == MyGUI::IntPoint(x, y))
				return;

			mMoveSpeedY = 0.1f * (y - mMoveStart.top);
			mMoveSpeedX = 0.1f * (x - mMoveStart.left);
		}
		else
		{
			BFG::v3 position;
			intersectPosition(widget, x, y, position);

			BFG::v3 startPosition;
			intersectPosition(widget, mDragStart.left, mDragStart.top, startPosition);

			BFG::v3 dragDifference = startPosition - position;

			const std::string camName(checkUserString(widget, "camHandle"));

			Ogre::SceneNode* node = mSceneMan->getSceneNode(camName);
			BFG::v3 nodePosition = BFG::View::toBFG(node->getPosition());
			BFG::v3 newPosition(nodePosition + dragDifference);
			node->setPosition(BFG::View::toOgre(newPosition));

			mDragStart = MyGUI::IntPoint(x, y);
		}
	}
	if (id == MyGUI::MouseButton::Middle)
	{
		if (mZoomStart.top == y)
			return;

		mZoomSpeed = 0.1f * (y - mZoomStart.top);
	}
	if (id == MyGUI::MouseButton::Left)
	{
		if (look == "Free")
		{
			if (mRotateStart == MyGUI::IntPoint(x, y))
				return;

			mRotateSpeedX = 0.01f * (x - mRotateStart.left);
			mRotateSpeedY = 0.01f * (y - mRotateStart.top);
		}
	}
}

void CameraControl::onKeyReleased(MyGUI::Widget* widget, MyGUI::KeyCode key)
{
	if (key == MyGUI::KeyCode::F5)
	{
		disableMultiview(mMultiview);

		if (mMultiview)
		{
			const std::string camName(checkUserString(widget, "camHandle"));
			const std::string lookFrom(checkUserString(widget, "lookFrom"));
			const std::string startDistance(checkUserString(widget, "startDistance"));
			const std::string type(checkUserString(widget, "type"));

			mFullView->setUserString("camHandle", camName);
			mFullView->setUserString("lookFrom", lookFrom);
			mFullView->setUserString("startDistance", startDistance);
			mFullView->setUserString("type", type);

			MyGUI::TextBox* fullTitle = mFullView->findWidget("Title")->castType<MyGUI::TextBox>();
			MyGUI::TextBox* viewTitle = widget->findWidget("Title")->castType<MyGUI::TextBox>();

			fullTitle->setCaption(viewTitle->getCaption());
			fullTitle->setSize(viewTitle->getSize());

			Ogre::RenderTarget** target = *widget->getUserData<Ogre::RenderTarget**>();

			setCanvas(mFullView, BFG::destringify(camName), target);
			resize();
		}
		else
		{
			MyGUI::Widget* title = mFullView->findWidget("Title");
			if (title)
				title->setProperty("Caption", "");
			removeCanvasEvents(mFullView);
		}

		mMultiview = !mMultiview;
	}
}

void CameraControl::onKeyPressed(MyGUI::Widget* widget, MyGUI::KeyCode key, MyGUI::Char c)
{
	if (key == MyGUI::KeyCode::None && c == 99)
	{
		mCreationMode = !mCreationMode;
		
		for (size_t i = 0; i < 4; ++i)
		{
			if (mCreationMode)
			{
				mViews[i]->findWidget("Debug")->setProperty("Caption", "create");
			}
			else
			{
				mViews[i]->findWidget("Debug")->setProperty("Caption", "");
			}
		}
	}
}

void CameraControl::update(const Ogre::FrameEvent& evt)
{
	if (!mCamerasCreated)
	{
		if (mSceneMan->hasCamera(stringify(mData->mCameras[0])) &&
		    mSceneMan->hasCamera(stringify(mData->mCameras[1])) &&
			mSceneMan->hasCamera(stringify(mData->mCameras[2])) &&
			mSceneMan->hasCamera(stringify(mData->mCameras[3])))
		{
			mCamerasCreated = true;
			resize();
			for (size_t i = 0; i < 4; ++i)
			{
				if (mViews[i]->getUserString("type") == "orthographic")
				{
					const std::string distance(mViews[i]->getUserString("startDistance"));
					Ogre::Camera* cam = mSceneMan->getCamera(stringify(mData->mCameras[i]));
					cam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
					cam->setOrthoWindowHeight(MyGUI::utility::parseFloat(distance));
				}
			}
		}
	}

	if (mFullSize != mFullView->getSize())
		resize();

	if (mZoomWidget)
	{
		if (mZoomSpeed > BFG::EPSILON_F || mZoomSpeed < -BFG::EPSILON_F)
		{
			const std::string camName(checkUserString(mZoomWidget, "camHandle"));
			const std::string look(checkUserString(mZoomWidget, "lookFrom"));
			const std::string camType(checkUserString(mZoomWidget, "type"));

			if (camType == "orthographic")
			{
				Ogre::Camera* cam = mSceneMan->getCamera(camName);
				f32 height = cam->getOrthoWindowHeight();

				height += mZoomSpeed * evt.timeSinceLastFrame;
				if (height < BFG::EPSILON_F)
					height = BFG::EPSILON_F;

				cam->setOrthoWindowHeight(height);
			}
			else if (camType == "perspective")
			{
				Ogre::SceneNode* node = mSceneMan->getSceneNode(camName);

				node->translate(0, 0, mZoomSpeed * evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);

				ViewParameter& vp = mViewParameterMap[look];
				vp.mPlane.d += mZoomSpeed * evt.timeSinceLastFrame;
			}
		}
	}
	if (mMoveWidget)
	{
		const std::string camName(checkUserString(mMoveWidget, "camHandle"));
	
		Ogre::SceneNode* node = mSceneMan->getSceneNode(camName);

		node->translate
		(
			mMoveSpeedX * evt.timeSinceLastFrame, 
			mMoveSpeedY * evt.timeSinceLastFrame, 
			0, 
			Ogre::Node::TS_LOCAL
		);
	}
	if (mRotateWidget)
	{
		const std::string camName(checkUserString(mRotateWidget, "camHandle"));
	
		Ogre::SceneNode* node = mSceneMan->getSceneNode(camName);

		node->pitch(Ogre::Radian(mRotateSpeedY * evt.timeSinceLastFrame));
		node->yaw(-Ogre::Radian(mRotateSpeedX * evt.timeSinceLastFrame));
	}
}

} // namespace Tool
