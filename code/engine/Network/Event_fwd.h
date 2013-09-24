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

#ifndef BFG_NETWORK_EVENT_FWD_H
#define BFG_NETWORK_EVENT_FWD_H

#include <boost/tuple/tuple.hpp>

#include <Core/CharArray.h>
#include <Core/Types.h>

#include <Network/Enums.hh>

namespace BFG {
namespace Network {

struct DataPayload;

//! \note
//! The Destination and Sender handles, used for the event delivery itself, are
//! very different to use from their application handle counterparts within the
//! struct Network::DataPayload.
//!
//! NE_SEND as Server:
//! -> Destination: Must be Destination PeerId or 0 f. broadcast
//! -> Sender: Ignored
//!
//! NE_SEND as Client:
//! -> Destination: Must be always 0
//! -> Sender: Ignored
//!
//! NE_RECEIVED as Server:
//! -> Destination: Same as NetworkPayload::mAppDestination
//! -> Sender: Remote PeerId
//!
//! NE_RECEIVED as Client:
//! -> Destination: Same as NetworkPayload::mAppDestination
//! -> Sender: Always 0

typedef boost::tuple
<
	CharArray128T,      // ip
	CharArray128T       // port
> EndpointT;

} // namespace Network
} // namespace BFG

#endif
