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

#include <View/ControllerMyGuiAdapter.h>

#include <boost/foreach.hpp>
#include <MyGUI.h>
#include <OISMouse.h>

#include <Controller/ControllerEvents.h>
#include <Controller/OISUtils.h>

namespace BFG {
namespace View {

using BFG::f32;

ControllerMyGuiAdapter::ControllerMyGuiAdapter(BFG::GameHandle stateHandle,
                                               Event::Lane* lane) :
mSubLane(lane->createSubLane())
{
	mSubLane->connect(ID::A_MOUSE_MOVE_X, this, &ControllerMyGuiAdapter::onMouseMoveX);
	mSubLane->connect(ID::A_MOUSE_MOVE_Y, this, &ControllerMyGuiAdapter::onMouseMoveY);
	mSubLane->connect(ID::A_MOUSE_MOVE_Z, this, &ControllerMyGuiAdapter::onMouseMoveZ);
	mSubLane->connect(ID::A_MOUSE_LEFT_PRESSED, this, &ControllerMyGuiAdapter::onMouseLeftPressed);
	mSubLane->connect(ID::A_MOUSE_RIGHT_PRESSED, this, &ControllerMyGuiAdapter::onMouseRightPressed);
	mSubLane->connect(ID::A_MOUSE_MIDDLE_PRESSED, this, &ControllerMyGuiAdapter::onMouseMiddlePressed);
	mSubLane->connect(ID::A_KEY_PRESSED, this, &ControllerMyGuiAdapter::onKeyPressed);
	mSubLane->connect(ID::A_KEY_RELEASED, this, &ControllerMyGuiAdapter::onKeyReleased);
}

void ControllerMyGuiAdapter::onMouseMoveX(f32 value)
{
	mMouseBuffer.x = value;
	MyGUI::InputManager::getInstance().injectMouseMove
	(
		mMouseBuffer.x,
		mMouseBuffer.y,
		mMouseBuffer.z
	);
}

void ControllerMyGuiAdapter::onMouseMoveY(f32 value)
{
	mMouseBuffer.y = value;
	MyGUI::InputManager::getInstance().injectMouseMove
	(
		mMouseBuffer.x,
		mMouseBuffer.y,
		mMouseBuffer.z
	);
}

void ControllerMyGuiAdapter::onMouseMoveZ(f32 value)
{
	mMouseBuffer.z = value;
	MyGUI::InputManager::getInstance().injectMouseMove
	(
		mMouseBuffer.x,
		mMouseBuffer.y,
		mMouseBuffer.z
	);
}

void ControllerMyGuiAdapter::onMouseLeftPressed(bool isPressed)
{
	if (isPressed)
	{
		MyGUI::InputManager::getInstance().injectMousePress
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			MyGUI::MouseButton::Enum(OIS::MB_Left)
		);
	}
	else
	{
		MyGUI::InputManager::getInstance().injectMouseRelease
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			MyGUI::MouseButton::Enum(OIS::MB_Left)
		);
	}
}

void ControllerMyGuiAdapter::onMouseRightPressed(bool isPressed)
{
	if (isPressed)
	{
		MyGUI::InputManager::getInstance().injectMousePress
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			MyGUI::MouseButton::Enum(OIS::MB_Right)
		);
	}
	else
	{
		MyGUI::InputManager::getInstance().injectMouseRelease
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			MyGUI::MouseButton::Enum(OIS::MB_Right)
		);
	}
}

void ControllerMyGuiAdapter::onMouseMiddlePressed(bool isPressed)
{
	if (isPressed)
	{
		MyGUI::InputManager::getInstance().injectMousePress
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			MyGUI::MouseButton::Enum(OIS::MB_Middle)
		);
	}
	else
	{
		MyGUI::InputManager::getInstance().injectMouseRelease
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			MyGUI::MouseButton::Enum(OIS::MB_Middle)
		);
	}
}

void ControllerMyGuiAdapter::onKeyPressed(s32 key)
{
	ID::KeyboardButton code = (ID::KeyboardButton) key;

	OIS::KeyCode ois_key;
	BFG::s32 ois_ch;

	Controller_::Utils::convertToBrainDamagedOisStyle(code, ois_key, ois_ch);
	MyGUI::KeyCode::Enum mygui_key = (MyGUI::KeyCode::Enum) ois_key;
	MyGUI::Char mygui_ch = ois_ch;

	MyGUI::InputManager::getInstance().injectKeyPress(mygui_key, mygui_ch);
}

void ControllerMyGuiAdapter::onKeyReleased(s32 key)
{
	BFG::ID::KeyboardButton code = (BFG::ID::KeyboardButton) key;

	OIS::KeyCode ois_key;
	s32 ois_ch;

	Controller_::Utils::convertToBrainDamagedOisStyle(code, ois_key, ois_ch);
	MyGUI::KeyCode::Enum mygui_key = (MyGUI::KeyCode::Enum) ois_key;
	MyGUI::Char mygui_ch = ois_ch;

	if (mygui_key == MyGUI::KeyCode::None)
		mygui_key = MyGUI::KeyCode::Enum(mygui_ch);

	MyGUI::InputManager::getInstance().injectKeyRelease(mygui_key);
}

} // namespace View
} // namespace BFG
