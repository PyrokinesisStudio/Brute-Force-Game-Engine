/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#ifndef BFG_MODEL_STATE_H
#define BFG_MODEL_STATE_H

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

#include <Event/Event.h>

#include <Model/Defs.h>

#include <Core/Types.h>
#include <Core/Units.h>

namespace BFG {

class MODEL_API State
{
public:
	State(Event::Lane&);
	virtual ~State();

	//! \brief Periodically called function for application timing.
	//! Most games have a game loop, in which the most important modules
	//! have an update function. The following is such an update function,
	//! but it's not called by the game loop - the event system takes care
	//! of it by simply sending a "LoopEvent" at the right time.
	//!
	//! So what we're doing here is emulating OGRE's timeSinceLastFrame
	//! variable which is a delta to the previous absolute time. Also, we
	//! check if we decided to end the update process of this module, and
	//! notice the event system if true.
	virtual void onTick(const TimeT timeSinceLastTick) = 0;

	//! \brief Calling this will hold the update process of this State.
	//! No further events might be received after this. It is the proper
	//! way to destroy the state nebst calling the destructor.
	void stopUpdates();
	
private:
	void onLoop(Event::TickData tickData);
};

} // namespace BFG

#endif
