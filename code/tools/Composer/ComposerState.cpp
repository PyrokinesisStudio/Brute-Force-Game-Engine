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


#include <ComposerState.h>

#include <Core/Path.h>
#include <Controller/Action.h>
#include <Controller/StateInsertion.h>
#include <Controller/Enums.hh>
#include <View/WindowAttributes.h>

#include <Actions.h>

ComposerState::ComposerState(GameHandle handle, Event::Lane& lane) :
	State(lane),
	mLane(lane)
{
	initController();

	mLane.connect(A_QUIT, this, &ComposerState::onQuit);
	mLane.connectV(A_SCREENSHOT, this, &ComposerState::onScreenshot);
}


void ComposerState::onQuit(s32)
{
	mLane.emit(ID::EA_FINISH, Event::Void());
}

void ComposerState::initController()
{
	Controller_::ActionMapT actions;
	actions[A_QUIT] = "A_QUIT";
	actions[A_LOADING_MESH] = "A_LOADING_MESH";
	actions[A_LOADING_MATERIAL] = "A_LOADING_MATERIAL";
	actions[A_CAMERA_AXIS_X] = "A_CAMERA_AXIS_X";
	actions[A_CAMERA_AXIS_Y] = "A_CAMERA_AXIS_Y";
	actions[A_CAMERA_AXIS_Z] = "A_CAMERA_AXIS_Z";
	actions[A_CAMERA_MOVE] = "A_CAMERA_MOVE";
	actions[A_CAMERA_RESET] = "A_CAMERA_RESET";
	actions[A_CAMERA_ORBIT] = "A_CAMERA_ORBIT";
	actions[A_SCREENSHOT] = "A_SCREENSHOT";
	actions[A_LOADING_SKY] = "A_LOADING_SKY";

	actions[A_CREATE_LIGHT] = "A_CREATE_LIGHT";
	actions[A_DESTROY_LIGHT] = "A_DESTROY_LIGHT";
	actions[A_PREV_LIGHT] = "A_PREV_LIGHT";
	actions[A_NEXT_LIGHT] = "A_NEXT_LIGHT";
	actions[A_FIRST_LIGHT] = "A_FIRST_LIGHT";
	actions[A_LAST_LIGHT] = "A_LAST_LIGHT";

	actions[A_INFO_WINDOW] = "A_INFO_WINDOW";

	actions[A_CAMERA_MOUSE_X] = "A_CAMERA_MOUSE_X";
	actions[A_CAMERA_MOUSE_Y] = "A_CAMERA_MOUSE_Y";
	actions[A_CAMERA_MOUSE_Z] = "A_CAMERA_MOUSE_Z";
	actions[A_CAMERA_MOUSE_MOVE] = "A_CAMERA_MOUSE_MOVE";

	actions[A_SUB_MESH] = "A_SUB_MESH";
	actions[A_TEX_UNIT] = "A_TEX_UNIT";
	actions[A_ANIMATION] = "A_ANIMATION";
	actions[A_ADAPTER] = "A_ADAPTER";

	actions[A_UPDATE_FEATURES] = "A_UPDATE_FEATURES";

	Controller_::fillWithDefaultActions(actions);	
	Controller_::sendActionsToController(mLane, actions);

	Path path;
	
	const std::string config_path = path.Expand("Composer.xml");
	const std::string state_name = "Composer";
		
	View::WindowAttributes wa;
	View::queryWindowAttributes(wa);
		
	Controller_::StateInsertion si(config_path, state_name, generateHandle(), true, wa);

	mLane.emit(ID::CE_LOAD_STATE, si);
}
