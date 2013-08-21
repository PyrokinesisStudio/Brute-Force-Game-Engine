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


#ifndef SI_MAIN_STATE_H
#define SI_MAIN_STATE_H


#include <Core/ClockUtils.h>

#include <Controller/Enums.hh>

#include <Model/Environment.h>
#include <Model/Sector.h>
#include <Model/State.h>

#include "InvaderGeneral.h"
#include "HumanGeneral.h"
#include "ViewState.h"

using namespace BFG;

struct MainState : State
{
	MainState(GameHandle handle, Event::Lane& lane);

	void onTick(const TickTimeT timeSinceLastTick);
	
	void onAxisY(f32 factor);
	void onFps(bool on);
	void onShipFire();
	void onQuit(BFG::s32);

	GameHandle mPlayer;

	boost::shared_ptr<Sector> mSector;
	boost::shared_ptr<Environment> mEnvironment;

	InvaderGeneral mInvaderGeneral;
	HumanGeneral   mHumanGeneral;

	BFG::Property::PluginMapT mPluginMap;

	Event::Lane& mLane;
};

#endif
