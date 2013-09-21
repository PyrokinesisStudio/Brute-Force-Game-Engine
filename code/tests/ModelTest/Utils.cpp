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

#include <Model/Environment.h>

GoAndSpacePluginT createTestGameObject(BFG::Event::Lane& lane,
                                       BFG::Property::PluginMapT& pluginMap,
                                       BFG::GameHandle handle,
                                       const BFG::Module::ValueStorageT& vs)
{
	BFG::PluginId spId = BFG::Property::ValueId::ENGINE_PLUGIN_ID;
	boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
	pluginMap.insert(sp);

	boost::shared_ptr<BFG::Environment> environment(new BFG::Environment());
	boost::shared_ptr<BFG::GameObject> go(new BFG::GameObject(
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
