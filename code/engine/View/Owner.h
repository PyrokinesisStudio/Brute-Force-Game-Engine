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

#ifndef BFG_VIEW_OWNER_H
#define BFG_VIEW_OWNER_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <EventSystem/Event_fwd.h>
#include <View/Enums.hh>
#include <View/Event_fwd.h>
#include <View/Light.h>

class EventLoop;

namespace BFG {
namespace View {

class RenderObject;
class Camera;
class Skybox;

class VIEW_API Owner
{
public:
	Owner(GameHandle stateHandle, EventLoop* loop);
	virtual ~Owner();

protected:
	void eventHandler(Event* VE);

	void createObject(const ObjectCreation& OC);
	void destroyObject(GameHandle handle);
	void createCamera(const CameraCreation& CC);
	void setSky(const SkyCreation& SC);
	void createLight(const LightParameters& LC);
	void setAmbient(const cv4& colour);

	typedef std::map<GameHandle, boost::shared_ptr<RenderObject> > ObjectMapT;
	typedef std::map<BFG::GameHandle, boost::shared_ptr<Light> > LightMapT;

	ObjectMapT                              mObjects;
	std::vector<boost::shared_ptr<Camera> > mCameras;
	boost::scoped_ptr<Skybox>               mSky;
	LightMapT                               mLights;
	
	EventLoop*                  mLoop;
	std::vector<ID::ViewAction> mViewEvents;
};


} // namespace View
} // namespace BFG
#endif
