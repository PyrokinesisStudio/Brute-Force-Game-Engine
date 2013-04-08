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

#ifndef BFG_NETWORK_CHECKSUM_H
#define BFG_NETWORK_CHECKSUM_H

#include <Network/Defs.h>
#include <Core/Types.h>

namespace BFG {
namespace Network {

struct Handshake;
struct NetworkEventHeader;

//! \brief Calculates the checksum of a data packet
//! \param[in] data The data part of the packet to calculate the checksum for
//! \param[in] length The length of the data part in bytes
//! \return Calculated checksum
NETWORK_API u32 calculateChecksum(const char* data, std::size_t length);

//! \brief Calculates the checksum of a NetworkEventHeader
//! \param[in] neh The NetworkEventHeader to calculate the checksum for
//! \return Calculated checksum
NETWORK_API u16 calculateHeaderChecksum(const NetworkEventHeader& neh);

//! \brief Calculates the checksum of a Handshake
//! \param[in] hs The Handshake to calculate the checksum for
//! \return Calculated checksum
NETWORK_API u16 calculateHandshakeChecksum(const Handshake& hs);

} // namespace Network
} // namespace BFG

#endif
