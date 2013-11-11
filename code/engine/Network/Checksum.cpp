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

#include <Network/Checksum.h>

#include <boost/crc.hpp>
#include <Network/Handshake.h>
#include <Network/Tcp.h>

namespace BFG {
namespace Network { 

u32 calculateChecksum(const char* data, std::size_t length)
{
	boost::crc_32_type result;
	result.process_bytes(data, length);
	return result.checksum();
}

u16 calculateHeaderChecksum(const NetworkEventHeader& neh)
{
       boost::crc_16_type result;
       result.process_bytes(&(neh.mDataLength), sizeof(neh.mDataLength));
       result.process_bytes(&(neh.mTimestamp), sizeof(neh.mTimestamp));
       result.process_bytes(&(neh.mDataChecksum), sizeof(neh.mDataChecksum));
       return result.checksum();
}

u16 calculateHandshakeChecksum(const Handshake& hs)
{
	boost::crc_16_type result;
	result.process_bytes(&hs.mPeerId, sizeof(PeerIdT));
	return result.checksum();
}

} // namespace Network
} // namespace BFG

