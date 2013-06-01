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

#include <Model/State.h>

#include <Core/ClockUtils.h>
#include <Core/Types.h>

using namespace boost::units;

namespace BFG {

State::State(EventLoop* loop) :
Emitter(loop),
mClock(new BFG::Clock::StopWatch(BFG::Clock::milliSecond)),
mExitNextTick(false)
{
	// Setting up callbacks for the game loop, only those which must be
	// updated regularily by a LoopEvent
	loop->registerLoopEventListener(this, &State::LoopEventHandler);	

	// Start the timing mechanism for onLoop().
	mClock->start();
}

State::~State()
{
	stopUpdates();
}

void State::LoopEventHandler(LoopEvent* e)
{
	if (mExitNextTick)
		e->data().getLoop()->setExitFlag();

	long timeSinceLastFrame = mClock->stop();
	if (timeSinceLastFrame)
		mClock->start();

	f32 timeInSeconds = static_cast<f32>(timeSinceLastFrame) / BFG::Clock::milliSecond;

	quantity<si::time, f32> TSLF = timeInSeconds * si::seconds;

	onTick(TSLF);
}

void State::stopUpdates()
{
	loop()->setExitFlag();
	loop()->unregisterLoopEventListener(this);
}

} // namespace BFG

