/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

#include "Utils.h"

#include <Core/Types.h>
#include <Model/Environment.h>

using namespace BFG;

GoAndSpacePluginT createTestGameObject(Event::Lane& lane,
                                       Property::PluginMapT& pluginMap,
                                       GameHandle handle,
                                       const Module::ValueStorageT& vs)
{
	PluginId spId = Property::ValueId::ENGINE_PLUGIN_ID;
	boost::shared_ptr<SpacePlugin> sp(new SpacePlugin(spId));
	pluginMap.insert(sp);

	boost::shared_ptr<Environment> environment(new Environment());
	boost::shared_ptr<GameObject> go(new GameObject(
		lane,
		handle,
		"TestObject",
		vs,
		pluginMap,
		environment
	));

	GoAndSpacePluginT ret = std::make_pair(go, sp);
	return ret;
}

Physics::FullSyncData createTestFullSyncData()
{
	const v3 pos(0.1f, 0.2f, 0.3f);
	const qv4 ori(0.4f, 0.5f, 0.6f);
	const v3 vel(0.7f, 0.8f, 0.9f);
	const v3 relvel(1.2f, 1.5f, 1.8f);
	const v3 rotvel(2.1f, 2.4f, 2.7f);
	const v3 relrotvel(3.0f, 3.3f, 3.6f);
	const f32 mass = 999.0f;
	const m3x3 inertia(9.f,8.f,7.f,6.f,5.f,4.f,3.f,2.f,1.f);
	
	Physics::FullSyncData fsd(pos, ori, vel, relvel, rotvel, relrotvel, mass, inertia);
	return fsd;
}