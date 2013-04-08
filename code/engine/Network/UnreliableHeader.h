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

#ifndef BFG_UNRELIABLE_HEADER_H
#define BFG_UNRELIABLE_HEADER_H

#include <cstring>
#include <boost/array.hpp>

namespace BFG {
namespace Network {

struct UnreliableHeader
{
	typedef boost::array<char, 6> SerializationT;

	void serialize(SerializationT& output) const
	{
		char* p = output.data();
		memcpy(p, &mTimestamp, sizeof(mTimestamp));
		p += sizeof(mTimestamp);
		memcpy(p, &mSequenceNumber, sizeof(mSequenceNumber));
	}

	void deserialize(const SerializationT& input)
	{
		const char* p = input.data();
		memcpy(&mTimestamp, p, sizeof(mTimestamp));
		p += sizeof(mTimestamp);
		memcpy(&mSequenceNumber, p, sizeof(mSequenceNumber));
	}

	// set biggest datatype to the top to optimize padding
	f32 mTimestamp;
	u16 mSequenceNumber;
};

} // namespace Network
} // namespace BFG

#endif
