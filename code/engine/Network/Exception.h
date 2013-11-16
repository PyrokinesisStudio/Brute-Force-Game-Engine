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

#ifndef BFG_NETWORK_EXCEPTION_H
#define BFG_NETWORK_EXCEPTION_H

#include <stdexcept>
#include <boost/uuid/uuid.hpp>

namespace BFG {
namespace Network {

//! \todo These exceptions aren't done yet.

//! \brief A token couldn't be identified.
struct UnknownTokenException : std::runtime_error
{
	UnknownTokenException(boost::uuids::uuid token) :
	std::runtime_error("Unknown token")
	{}
};

//! \brief An endpoint couldn't be identified.
template <typename ProtocolT>
struct UnknownEndpointException : std::runtime_error
{
	UnknownEndpointException(typename ProtocolT::EndpointPtrT endpoint) :
	std::runtime_error("Unknown endpoint")
	{}
};

//! \brief A peer couldn't be identified.
struct UnknownPeerException : std::runtime_error
{
	UnknownPeerException() :
	std::runtime_error("Unknown peer")
	{}
};

//! \brief A token was expected within a packet, but not found.
struct NoTokenFoundException : std::runtime_error
{
	NoTokenFoundException() :
	std::runtime_error("No token found")
	{}
};

} // namespace Network
} // namespace BFG

#endif
