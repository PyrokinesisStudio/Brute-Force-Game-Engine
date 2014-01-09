/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2014 Brute-Force Games GbR

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

#ifndef CONTROLLER_ACTIONBUILDER_H
#define CONTROLLER_ACTIONBUILDER_H

#include <string>
#include <type_traits>
#include <boost/weak_ptr.hpp>
#include <Controller/Action.h>
#include <Controller/ControllerEvents_fwd.h>

namespace BFG {
namespace Controller_ {

//! Helper class for Controller_::State initialization.
//! \see createActionBuilder
//! \todo Maybe use SubLanePtr by default.
template <typename LanePtrT>
class ActionBuilder
{
public:
	ActionBuilder(LanePtrT lane) :
	mLane(lane)
	{}
	
	//! Adds all raw input events from the Controller
	void addDefaultActions()
	{
		auto first = ID::A_FIRST_CONTROLLER_ACTION + 1;
		auto last = ID::A_LAST_CONTROLLER_ACTION;
		for (auto i = first; i < last; ++i)
		{
			auto ca = static_cast<ID::ControllerAction>(i);
			mActions[i] = ID::asStr(ca);
		}
	}
	
	//! Adds a single action
	//! \param[in] eventId The ID
	//! \param[in] eventIdStr The serialized ID
	template <typename EventIdT>
	void add(EventIdT eventId, std::string eventIdStr)
	{
		mActions[eventId] = eventIdStr;
	}

	//! Adds the content of another ActionMapT to this one.
	void add(const ActionMapT& actionMap)
	{
		mActions.insert(actionMap.begin(), actionMap.end());
	}
	
	//! Sends all actions to the controller. Call this at the end.
	void emitActions() const
	{
		for (auto& idAndString : mActions)
		{
			mLane->emit
			(
				ID::CE_ADD_ACTION,
				ActionDefinition
				(
					idAndString.first,
					stringToArray<128>(idAndString.second)
				)
			);
		}
	}
	
	LanePtrT mLane;
	ActionMapT mActions;
};

//! Use this to create an ActionBuilder
template <typename LanePtrT>
ActionBuilder<LanePtrT>
createActionBuilder(LanePtrT& lane)
{
	ActionBuilder<LanePtrT> ab(lane);
	return ab;
}

} // namespace Controller_
} // namespace BFG

#endif

