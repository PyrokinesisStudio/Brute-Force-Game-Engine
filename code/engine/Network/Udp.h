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

#ifndef BFG_NETWORK_UDP_H
#define BFG_NETWORK_UDP_H

#include <boost/asio/ip/udp.hpp>

#include <Network/Enums.hh>
#include <Network/UnreliableHeader.h>

namespace BFG {
namespace Network {

class UdpHeaderFactory
{
	typedef UnreliableHeader HeaderT;

public:
	UdpHeaderFactory() :
	mSequenceNumber(0)
	{}
	
	//! Creates an UnreliableHeader in a provided buffer.
	//! \param[in] buffer The buffer to write the header data in
	UnreliableHeader create(boost::asio::const_buffer, std::size_t) const
	{
		++mSequenceNumber;
		
		// Make header
		UnreliableHeader header;
		header.mSequenceNumber = mSequenceNumber;
		
		// TODO: Create Timestamp
		header.mTimestamp = 0.0f;
		return header;
	}
	
private:
	mutable BFG::u32 mSequenceNumber;
};

struct Udp
{
	static const std::size_t MAX_PACKET_SIZE_BYTES = 1400;
	static const ID::NetworkAction EVENT_ID_FOR_SENDING = ID::NE_SEND_UDP;

	//! Max size a packet can expand to before it will be flushed (Q3: rate)
	static const u32 MAX_BYTE_RATE = 100000;
	
	typedef UnreliableHeader HeaderT;
	typedef UdpHeaderFactory HeaderFactoryT;

	typedef boost::asio::ip::udp::endpoint EndpointT;
	typedef boost::asio::ip::udp::socket SocketT;
	
	static std::size_t headerSize()
	{
		return HeaderT::SerializationT::size();
	}
};

} // namespace Network
} // namespace BFG

#endif
