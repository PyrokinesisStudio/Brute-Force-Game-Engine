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

#include <Controller/ControllerImpl.h>

#include <Base/Logger.h>
#include <Core/ClockUtils.h>

#include <Controller/State.h>
#include <Controller/ControllerEvents_fwd.h>
#include <Controller/StateInsertion.h>

namespace BFG {
namespace Controller_ {

namespace posix_time = boost::posix_time;

//#############################################################################
//                        The almighty Controller
//#############################################################################

Controller::Controller(Event::Lane& eventLane) :
mEventLane(eventLane)
{
	eventLane.connectLoop(this, &Controller::loopHandler);
	
	eventLane.connect(ID::CE_ADD_ACTION, this, &Controller::addAction);
	eventLane.connect(ID::CE_LOAD_STATE, this, &Controller::insertState);
	eventLane.connect(ID::CE_ACTIVATE_STATE, this, &Controller::activateState);
	eventLane.connect(ID::CE_DEACTIVATE_STATE, this, &Controller::deactivateState);
}

Controller::~Controller()
{}

void Controller::init(int maxFrameratePerSec)
{
	dbglog << "Controller: Initializing with "
	       << maxFrameratePerSec << " FPS";

	if (maxFrameratePerSec <= 0)
		throw std::logic_error("Controller: Invalid maxFrameratePerSec param");

	mClock.reset
	(
		new Clock::SleepFrequently
		(
			Clock::microSecond,
			ONE_SEC_IN_MICROSECS / maxFrameratePerSec
		)
	);
}

void Controller::insertState(const StateInsertion& si)
{
	std::string name            = si.mStateName.data();
	std::string config_filename = si.mConfigurationFilename.data();

	dbglog << "Controller: Inserting state \"" << name << "\"";

	boost::shared_ptr<State> state(new State(mEventLane));

	mStates.insert(std::make_pair(si.mHandle, state));

	state->init
	(
		std::string(name),
		std::string(config_filename),
		mActions,
		si.mWindowAttributes.mWidth,
		si.mWindowAttributes.mHeight,
		si.mWindowAttributes.mHandle
	);

	if (si.mActivateNow)
		activateState(si.mHandle);
}

void Controller::removeState(GameHandle state)
{
	dbglog << "Controller: Removing State \"" << state << "\"";

	StateContainerT::iterator it = mStates.find(state);

	if (it != mStates.end())
	{
		mStates.erase(it);
	}
}

void Controller::capture()
{
	StateContainerT::iterator it = mStates.begin();
	for (; it != mStates.end(); ++it)
		it->second->capture();
}

void Controller::sendFeedback(long microseconds_passed)
{
	StateContainerT::iterator it = mStates.begin();
	for (; it != mStates.end(); ++it)
		it->second->sendFeedback(microseconds_passed);
}

void Controller::activateState(GameHandle state)
{
	dbglog << "Controller: Activating State \"" << state << "\"";

	StateContainerT::const_iterator it = mStates.find(state);

	if (it == mStates.end())
		errlog << "Controller: Unable to activate state \"" << state << "\"";

	it->second->activate();
}

void Controller::deactivateState(GameHandle state)
{
	dbglog << "Controller: Deactivating State \"" << state << "\"";

	StateContainerT::const_iterator it = mStates.find(state);

	if (it == mStates.end())
		errlog << "Controller: Unable to deactivate state \"" << state << "\"";

	it->second->deactivate();
}

void Controller::addAction(const ActionDefinition& ad)
{
	mActions.insert
	(
		std::make_pair
		(
			ad.get<0>(),
			std::string(ad.get<1>().data())
		)
	);
}

void Controller::loopHandler(const Event::TickData td)
{
	//! \todo: Use td.mTimeSinceLastTick and adapt the whole Controller
	//!        timing logic.
	nextTick();
}

void Controller::nextTick()
{
	capture();
	sendFeedback(mClock->measure());
}

void Controller::resetInternalClock()
{
	mClock->reset();
}

} // namespace Controller_
} // namespace BFG
