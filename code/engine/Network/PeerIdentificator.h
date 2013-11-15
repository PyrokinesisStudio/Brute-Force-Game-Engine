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

#ifndef BFG_NETWORK_IDENTIFICATOR_H
#define BFG_NETWORK_IDENTIFICATOR_H

#include <Network/Defs.h>
#include <Network/Packet.h>
#include <Network/Udp.h>

namespace BFG {
namespace Network { 

class PeerIdentificator
{
public:
	PeerIdT operator()(Udp::EndpointPtrT ep, OPacket<Udp>& firstPacket)
	{
		//dbglog << "PeerIdentificator identifying: " << *ep;
		return identify(ep, firstPacket);
	}
	
protected:
	virtual PeerIdT identify(Udp::EndpointPtrT, OPacket<Udp>& firstPacket) = 0;
};

class TokenIdentificator : public PeerIdentificator
{
public:
	virtual PeerIdT identify(Udp::EndpointPtrT, OPacket<Udp>& firstPacket);

	TokenT generateToken(PeerIdT);
	
private:
	PeerIdT identifyByEndpoint(Udp::EndpointPtrT remoteEndpoint) const;
	PeerIdT identifyByToken(OPacket<Udp>& firstPacket) const;

	void cacheEndpoint(PeerIdT, Udp::EndpointPtrT);
	
	std::map<TokenT, PeerIdT> mGeneratedTokens;
	std::map<Udp::EndpointT, PeerIdT> mEndpointCache;
};

class OneToOneIdentificator : public PeerIdentificator
{
public:
	virtual PeerIdT identify(Udp::EndpointPtrT, OPacket<Udp>& firstPacket)
	{
		return UNIQUE_PEER;
	}
};

} // namespace Network
} // namespace BFG

#endif
