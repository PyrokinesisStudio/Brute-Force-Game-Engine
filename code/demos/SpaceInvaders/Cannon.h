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

#ifndef SI_CANNON_H
#define SI_CANNON_H

#include <Core/Types.h>

#include <Event/Event.h>

#include <Model/GameObject.h>
#include <Model/Property/Concept.h>

using namespace BFG;

class Cannon : public Property::Concept
{
public:
	Cannon(GameObject& Owner, BFG::PluginId pid);

	void onFireRocket();
	void onPowerUp(s32 ammo);

	void fireRocket(bool autoRocket);
	
	s32 mAutoRocketAmmo;

private:
	std::string mLaserSound;
};

#endif
