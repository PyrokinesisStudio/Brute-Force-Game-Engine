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

#ifndef MODELTEST_UTILS_H
#define MODELTEST_UTILS_H

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <Core/GameHandle.h>
#include <Event/Lane.h>
#include <Model/GameObject.h>
#include <Model/Property/SpacePlugin.h>

typedef std::pair <
	boost::shared_ptr<BFG::GameObject>,
	boost::shared_ptr<BFG::SpacePlugin>
> GoAndSpacePluginT;

GoAndSpacePluginT createTestGameObject(BFG::Event::Lane& lane,
                                       BFG::Property::PluginMapT&,
                                       BFG::GameHandle = BFG::generateHandle(),
                                       const BFG::Module::ValueStorageT& vs = BFG::Module::ValueStorageT());

template<typename T>
void setValueId(boost::shared_ptr<BFG::Module> module, BFG::ID::PropertyVar id, T value);

#endif
