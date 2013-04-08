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

#ifndef BFG_NETWORK_HANDSHAKE_H
#define BFG_NETWORK_HANDSHAKE_H

#include <cstring>
#include <boost/array.hpp>
#include <Core/Types.h>
#include <Network/Defs.h>

namespace BFG {
namespace Network { 

struct Handshake
{
	typedef boost::array<char, 16> SerializationT;
	
	void serialize(SerializationT& output) const
	{
		char* p = output.data();
		memcpy(p, &mPeerId, sizeof(PeerIdT));
		p += sizeof(PeerIdT);
		memcpy(p, &mProtocolVersion, sizeof(u16));
		p += sizeof(u16);
		memcpy(p, &mChecksum, sizeof(u16));
	}

	void deserialize(const SerializationT& input)
	{
		const char* p = input.data();
		memcpy(&mPeerId, p, sizeof(PeerIdT));
		p += sizeof(PeerIdT);
		memcpy(&mProtocolVersion, p, sizeof(u16));
		p += sizeof(u16);
		memcpy(&mChecksum, p, sizeof(u16));
	}
	
	// TODO: replace PeerIdT with simpler type
	PeerIdT mPeerId;

	u16 mProtocolVersion;
	u16 mChecksum;
};


} // namespace Network
} // namespace BFG

#endif
