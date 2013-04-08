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

#ifndef BFG_NETWORK_PRINTERRORCODE_H
#define BFG_NETWORK_PRINTERRORCODE_H

#include <boost/asio/error.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <Base/Logger.h>
#include <Network/Defs.h>

namespace BFG {
namespace Network { 

//! \brief Logs an error_code
//! \param[in] ec Error code to log
//! \param[in] method Name of the method that received the error
//! \param[in] peerId Related peerId
inline void printErrorCode(const boost::system::error_code &ec, const std::string& method, PeerIdT peerId)
{
	std::string peerIdStr;
	if (peerId == UNIQUE_PEER)
		peerIdStr = "UNIQUE_PEER";
	else
		peerIdStr = boost::lexical_cast<std::string>(peerId);
	
	warnlog << "Network error: PeerID #" << peerIdStr
	        << " [" << method
	        << "] Error Code: " << ec.value()
	        << ", message: " << ec.message();
}

} // namespace Network
} // namespace BFG

#endif
