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

#ifndef BFG_NETWORK_UDPREADMODULE_H
#define BFG_NETWORK_UDPREADMODULE_H

#include <boost/function.hpp>

#include <Network/NetworkModule.h>
#include <Network/Udp.h>

namespace BFG {
namespace Network { 

class UdpReadModule : public NetworkModule<Udp>
{
public:
	typedef boost::function<PeerIdT(const boost::shared_ptr<Udp::EndpointT>)> EndpointIdentificatorT;
	
	UdpReadModule(EventLoop* loop_,
	              boost::asio::io_service& service,
	              boost::shared_ptr<Clock::StopWatch> localTime,
	              boost::shared_ptr<Udp::SocketT> socket,
	              EndpointIdentificatorT endpointIdentificator);
	
	virtual ~UdpReadModule();

	//! \brief Returns the socket of the connection
	//! \return socket of the connection
	boost::shared_ptr<Udp::SocketT> socket()
	{
		return mSocket;
	}
	
	virtual void startReading()
	{
		NetworkModule<Udp>::startReading();
	}
	
private:
	virtual void write(boost::asio::const_buffer, std::size_t)
	{
		assert(! "Useless call to UdpReadModule::write()");
	}
	
	virtual void read();

	void readHandler(const boost::system::error_code &ec, std::size_t bytesTransferred, boost::shared_ptr<Udp::EndpointT> remoteEndpoint);
	
	//! \brief Received data from the net is packed as a corresponding event 
	//! \param[in] data data array received from the network
	//! \param[in] size size of the data received
	virtual void onReceive(OPacket<Udp>& oPacket, PeerIdT peerId);
	
	boost::shared_ptr<Udp::SocketT> mSocket;

	// TODO: Use CreateBuffer
	boost::array<char, Udp::MAX_BYTE_RATE> mReadBuffer;
	
	EndpointIdentificatorT mEndpointIdentificator;
	
	// TODO: Do this in own class
	std::map<PeerIdT, BFG::u32> mLastSequenceNumbers;
};

} // namespace Network
} // namespace BFG

#endif
