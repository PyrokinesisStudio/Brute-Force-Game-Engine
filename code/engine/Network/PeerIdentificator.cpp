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

#include <Network/PeerIdentificator.h>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <Base/Logger.h>
#include <Network/Exception.h>

namespace BFG {
namespace Network { 

PeerIdT TokenIdentificator::identify(Udp::EndpointPtrT remoteEndpoint,
                                     OPacket<Udp>& firstPacket)
{
	dbglog << "Identifying " << *remoteEndpoint;
	try
	{
		return identifyByEndpoint(remoteEndpoint);
	}
	catch (UnknownEndpointException<Udp>& ex)
	{
		auto peerId = identifyByToken(firstPacket);
		cacheEndpoint(peerId, remoteEndpoint);
		return peerId;
	}
}

TokenT TokenIdentificator::generateToken(PeerIdT peerId)
{
	auto token = boost::uuids::random_generator()();
	mGeneratedTokens[token] = peerId;
	return token;
}

PeerIdT TokenIdentificator::identifyByEndpoint(Udp::EndpointPtrT remoteEndpoint) const
{
	// TODO: remove Debug stuff
	for (auto pair : mEndpointCache) {
		warnlog << pair.second << " -> " << pair.first;
	}
	
	auto ep = *remoteEndpoint;
	auto it = mEndpointCache.find(ep);
	if (it == mEndpointCache.end())
		throw UnknownEndpointException<Udp>(remoteEndpoint);
	else
		return it->second;
}

PeerIdT TokenIdentificator::identifyByToken(OPacket<Udp>& firstPacket) const
{
	// TODO: Use real Factory!
	auto dummy1 = boost::make_shared<Clock::StopWatch>(Clock::milliSecond);
	dummy1->start();
	auto dummy2 = PayloadFactory(0,dummy1, Rtt<s32,10>());
	
	auto firstPayload = firstPacket.nextPayload(dummy2);

	// Extract identification token
	auto token = boost::uuids::uuid();
	std::copy
	(
		firstPayload.mAppData.begin(),
		firstPayload.mAppData.begin() + token.size(),
		token.begin()
	);

	// Identify peer via token
	auto it = mGeneratedTokens.find(token);
	if (it == mGeneratedTokens.end())
		throw UnknownTokenException(token);

	const PeerIdT peerId = it->second;
	
	dbglog << "PeerIdentificator: Identified peer " << peerId
	       << " with token: " << token;

	return peerId;
}

void TokenIdentificator::cacheEndpoint(PeerIdT peerId, Udp::EndpointPtrT remoteEndpoint)
{
	mEndpointCache[*remoteEndpoint] = peerId;
}

} // namespace Network
} // namespace BFG
