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

#include <Network/UdpWriteModule.h>

#include <boost/uuid/uuid_io.hpp>

#include <Base/Logger.h>
#include <Network/Defs.h>

namespace BFG {
namespace Network { 

namespace asio = boost::asio;

UdpWriteModule::UdpWriteModule(Event::Lane& lane,
                               asio::io_service& service,
                               PeerIdT peerId,
                               boost::shared_ptr<Clock::StopWatch> localTime,
                               boost::shared_ptr<Udp::SocketT> socket,
                               const boost::shared_ptr<Udp::EndpointT> remoteEndpoint) :
NetworkModule<Udp>(lane, service, peerId, localTime),
mSocket(socket),
mRemoteEndpoint(remoteEndpoint)
{
	dbglog << "Creating UdpWriteModule #" << peerId << " aka " << *remoteEndpoint;
}

UdpWriteModule::~UdpWriteModule()
{
	dbglog << "Destroying UdpWriteModule #" << mPeerId << " aka " << *mRemoteEndpoint;
}

void UdpWriteModule::pingRemote(TokenT udpToken)
{
	dbglog << "UdpWriteModule #" << mPeerId
	       << " pinging remote " << *mRemoteEndpoint
	       << " aka #" << mPeerId
	       << " with token: " << udpToken;
	
	CharArray512T ca512;
	std::copy(udpToken.begin(), udpToken.end(), ca512.begin());
	DataPayload payload(ID::NE_PING_UDP, 0, 0, udpToken.size(), ca512);
	onSend(payload);
}

void UdpWriteModule::write(asio::const_buffer packet, std::size_t size)
{
	dbglog << "UdpWriteModule::write() bytes: " << size;

	socket()->async_send_to
	(
		asio::buffer(packet),
		*mRemoteEndpoint,
		bind
		(
			&UdpWriteModule::writeHandler,
			boost::static_pointer_cast<UdpWriteModule>(shared_from_this()),
			_1,
			_2,
			packet
		)
	);
}

} // namespace Network
} // namespace BFG
