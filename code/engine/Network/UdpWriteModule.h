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

#ifndef BFG_NETWORK_UDPWRITEMODULE_H
#define BFG_NETWORK_UDPWRITEMODULE_H

#include <Network/NetworkModule.h>
#include <Network/Udp.h>

namespace BFG {
namespace Network { 

class UdpWriteModule : public NetworkModule<Udp>
{
public:
	UdpWriteModule(Event::Lane& lane,
	               boost::asio::io_service& service,
	               PeerIdT peerId,
	               boost::shared_ptr<Clock::StopWatch> localTime,
	               boost::shared_ptr<Udp::SocketT> socket,
	               const boost::shared_ptr<Udp::EndpointT> remoteEndpoint);
	
	virtual ~UdpWriteModule();

	//! \brief Returns the socket of the connection
	//! \return socket of the connection
	boost::shared_ptr<Udp::SocketT> socket()
	{
		return mSocket;
	}
	
	void pingRemote(TokenT);
	
	virtual void startSending()
	{
		NetworkModule<Udp>::startSending();
	}
	
private:
	//! \brief Perform an asynchronous write of data to the connected network module
	//! \param[in] packet data to write over the net
	//! \param[in] size Size of the data set
	virtual void write(boost::asio::const_buffer packet, std::size_t size);

	virtual void read()
	{
		assert(! "Useless call to UdpWriteModule::read()");
	}

	virtual void onReceive(OPacket<Udp>&, PeerIdT)
	{
		assert(! "Useless call to UdpWriteModule::onReceive()");
	}
	
	boost::shared_ptr<Udp::SocketT> mSocket;

	const boost::shared_ptr<boost::asio::ip::udp::endpoint> mRemoteEndpoint;
};

} // namespace Network
} // namespace BFG

#endif
