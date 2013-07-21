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

#ifndef CONTROLLER_EVENTS_FWD_H_
#define CONTROLLER_EVENTS_FWD_H_

#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>

#include <Core/CharArray.h>
#include <Core/GameHandle.h>
#include <Core/Types.h>
#include <Controller/Enums.hh>

#include <Event/Event.h>

namespace BFG {
namespace Controller_ {

struct StateInsertion;

//! Example: 2013 = "A_FIRE_PRIMARY_WEAPON"
typedef boost::tuple <
	Event::IdT,
	CharArray128T
> ActionDefinition;

typedef boost::variant <
	bool,
	s32,
	f32
> VipPayloadT;

} // namespace Controller_
} // namespace BFG

#endif //CONTROLLER_EVENTS_FWD_H_
