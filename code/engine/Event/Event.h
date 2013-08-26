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

#ifndef BFG_EVENT_EVENT_H
#define BFG_EVENT_EVENT_H

#include <Core/GameHandle.h>

#include <Event/Envelope.h>
#include <Event/Lane.h>
#include <Event/Synchronizer.h>
#include <Event/SubLane.h>

namespace BFG {
namespace Event {

typedef BasicEnvelope<u32, GameHandle, GameHandle> Envelope;
USING_ENVELOPE(Envelope);

typedef BasicLane<Envelope> Lane;
typedef BasicSynchronizer<Lane> Synchronizer;
typedef BasicSubLane<Lane> SubLane;

typedef boost::shared_ptr<SubLane> SubLanePtr;

} // namespace Event
} // namespace BFG

#endif
