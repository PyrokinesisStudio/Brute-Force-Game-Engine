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


#include "ViewState.h"

#include <Base/Cpp.h>
#include <Base/Logger.h>

#include <View/Main.h>
#include <View/EffectCreation.h>
#include <View/Explosion.h>

#include "PowerUpEffect.h"


ViewMainState::ViewMainState(GameHandle handle,  Event::Lane& lane) :
	State(handle, lane)
{
	lane.connect(ID::VE_EFFECT, this, &ViewMainState::onEffect);
}


ViewMainState::~ViewMainState()
{
	mLane.emit(BFG::ID::VE_SHUTDOWN, Event::Void());
}

bool ViewMainState::frameStarted(const Ogre::FrameEvent& evt)
{
	erase_if(mEffects, boost::bind(&View::Effect::done, _1));
	return true;
}


bool ViewMainState::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	return true;
}


bool ViewMainState::frameEnded(const Ogre::FrameEvent& evt)
{
	return true;
}

void ViewMainState::onEffect(const View::EffectCreation& ec)
{
	const std::string& ca = ec.get<0>();
	const v3& position = ec.get<1>();
	const f32 intensity = ec.get<2>();
	const std::string effect(ca.data());

	boost::shared_ptr<View::Effect> e;
	if (effect == "Explosion_1")
	{
		e.reset(new View::Explosion(position, intensity));
		mEffects.push_back(e);
	}
	else if(effect == "Explosion_2")
	{
		e.reset(new View::Explosion2(position, intensity));
		mEffects.push_back(e);
	}
	else if (effect == "Powerup_1")
	{
		e.reset(new PowerupEffect(position, intensity));
		mEffects.push_back(e);
	}
	else
		errlog << "ViewMainState::onEffect(): Got effect (\""
			   << effect
			   << "\")";
}
