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

#ifndef BFG_EVENTSYSTEM_GET_H
#define BFG_EVENTSYSTEM_GET_H

#include <sstream>
#include <stdexcept>
#include <typeinfo>

#include <boost/variant.hpp>

namespace BFG {

//! Wrapper function for calls to the data function of \ref BFG::Event with
//! Boost.Variant' boost::get providing better error information on failure.
//! \tparam T The expected type of the variant in e.
//! \tparam EventT Auto deduced event type of e.
//! \param[in] e An event with a variant as data type.
//! \return The content of the variant in e.data() as type T
//! \throw std::logic_error Contains a meaningful error message.
template <typename T, typename EventT>
T get(const EventT& e)
{
	try
	{
		return boost::get<T>(e.data());
	}
	catch (const boost::bad_get& ex)
	{
		// Construct error message
		std::stringstream ss;
		ss << "BFG::get: Catched boost::bad_get for"
		   << " Requested Type: \"" << typeid(T).name() << "\""
		   << " Real Type: \"" << e.data().type().name() << "\""
		   << " EventId: " << e.id()
		   << " Destination: " << e.destination()
		   << " Sender: " << e.sender();
		std::string msg = ss.str();
		
		throw std::logic_error(ss.str());
	}
}

} // namespace BFG

#endif
