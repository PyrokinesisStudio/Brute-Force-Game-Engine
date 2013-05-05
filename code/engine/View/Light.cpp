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

#include <View/Light.h>

#include <stdexcept>

#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <Core/GameHandle.h>

#include <View/Convert.h>
#include <View/Defs.h>
#include <View/LightParameters.h>

namespace BFG {
namespace View {

Light::Light(const LightParameters& LC) :
mHandle(LC.mHandle)
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);

	Ogre::Light* light = sceneMgr->createLight(stringify(LC.mHandle));

	light->setType(static_cast<Ogre::Light::LightTypes>(LC.mType));
	light->setDiffuseColour(LC.mDiffuseColor);
	light->setSpecularColour(LC.mSpecularColor);
	light->setPowerScale(LC.mPower);
	
	switch(LC.mType)
	{
		case ID::LT_Point:
			createPointLight(LC, light);
		break;

		case ID::LT_Directional:
			createDirectionalLight(LC, light);
		break;

		case ID::LT_Spot:
			createSpotLight(LC, light);
		break;

		default:
		throw std::logic_error("RaceState: onCreateLight: unknown LightType!");
	}
}

Light::~Light()
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->destroyLight(stringify(mHandle));
}

void Light::createSpotLight(const LightParameters& LC, Ogre::Light*& light)
{
	light->setPosition(toOgre(LC.mPosition));
	light->setDirection(toOgre(LC.mDirection));
	light->setAttenuation
	(
		LC.mRange,
		LC.mConstant,
		LC.mLinear,
		LC.mQuadric
	);
	light->setSpotlightRange
	(
		Ogre::Degree(LC.mInnerRadius),
		Ogre::Degree(LC.mOuterRadius),
		LC.mFalloff
	);
}

void Light::createDirectionalLight(const LightParameters& LC, Ogre::Light*& light)
{
	light->setDirection(toOgre(LC.mDirection));
}

void Light::createPointLight(const LightParameters& LC, Ogre::Light*& light)
{
	light->setPosition(toOgre(LC.mPosition));
	light->setAttenuation
	(
		LC.mRange,
		LC.mConstant,
		LC.mLinear,
		LC.mQuadric
	);
}

} // namespace View
} // namespace BFG
