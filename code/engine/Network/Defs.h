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

#ifndef BFG_NETWORKDEFS_H
#define BFG_NETWORKDEFS_H

#include <Core/Types.h>

#define BFG_SERVER 0
#define BFG_CLIENT 1

#ifdef _WIN32
	#ifndef NETWORK_EXPORTS
		#define NETWORK_API __declspec(dllimport)
	#else
		#define NETWORK_API __declspec(dllexport)
	#endif //NETWORK_EXPORTS
#else // UNIX
    #define NETWORK_API
#endif// UNIX

namespace BFG {
namespace Network{

typedef GameHandle PeerIdT;
typedef u32        TimestampT;

const u32 PACKET_MTU(2000); // max size a packet can expand to before it will be flushed (Q3: rate)
const u32 FLUSH_WAIT_TIME(20); // ms before automatic flush (Q3: 1000/cl_update_rate), "natural flush time" depends on bandwidth
const u32 TIME_SYNC_WAIT_TIME(10000); // ms between time synchronization
const u32 RANDOM_PORT(0); // passed to Boost.Asio as port number in order to open a random port
const u16 UDP_PAYLOAD_SIZE(1400); // maximum size for UDP datagrams

}// namespace BFG
}// namespace Network

#endif
