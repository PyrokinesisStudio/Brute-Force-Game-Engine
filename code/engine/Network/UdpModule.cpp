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

#include <Network/UdpModule.h>
#include <boost/asio.hpp>
#include <Base/Logger.h>
#include <Network/CreateBuffer.h>
#include <Network/Defs.h>
#include <Network/Packet.h>
#include <Network/Udp.h>
#include <Network/UnreliableHeader.h>

namespace BFG {
namespace Network {

using namespace boost::asio;
using namespace boost::asio::ip;

UdpModule::UdpModule(EventLoop* loop, io_service& service, u16 port) :
Emitter(loop),
mSocket(service, udp::endpoint(udp::v4(), port)),
mPool(UDP_PAYLOAD_SIZE)
{
	read();
	errlog << "UDP";
}

UdpModule::~UdpModule()
{
	errlog << "~UDP";
}

void UdpModule::read()
{
	errlog << "UDP: read() from: " << mRemoteEndpoint;
	char* buf = static_cast<char*>(mPool.malloc());
	
	mSocket.async_receive_from
	(
		buffer(buf, UDP_PAYLOAD_SIZE),
		mRemoteEndpoint,
		boost::bind(
			&UdpModule::readHandler,
			this,
			_1,
			_2,
			buf
		)
	);
}

void UdpModule::readHandler(const boost::system::error_code& ec, std::size_t bytesTransferred, char* buffer)
{
	errlog << "UDP: readHandler() ec: " << ec;
	errlog << "UDP: readHandler() got " << bytesTransferred << " bytes: " << std::string(buffer, bytesTransferred);
	mPool.free(buffer);

	// TEST PACKET
	char* sendBuffer = static_cast<char*>(mPool.malloc());
	Udp::HeaderFactoryT uhf;
	IPacket<Udp> p(createBuffer(mPool), uhf);
	
	// TEST DATA
	const char* data = "Hallo?";
	Segment segment;
	segment.appEventId = 12224;
	segment.destinationId = 1234567L;
	segment.senderId = 7654321L;
	segment.dataSize = strlen(data);

	// onSend()
	p.add(segment, data);
	
	write(p.full(), p.size());
}

void UdpModule::write(boost::asio::const_buffer buf, std::size_t bytesTransferred)
{
	errlog << "UDP: read()";
//	char* buf = static_cast<char*>(mPool.malloc());
	
	size_t SIZE = bytesTransferred;
	errlog << "write() SIZE: " << SIZE;
//	memset(buf, 0, UDP_PAYLOAD_SIZE);
//	strncpy(buf, "Hallo!", SIZE);
	
	mSocket.async_send_to
	(
		buffer(buf, SIZE),
		mRemoteEndpoint,
		boost::bind(
			&UdpModule::writeHandler,
			this,
			_1,
			_2,
			buf
		)
	);
}

void UdpModule::writeHandler(const boost::system::error_code &ec, std::size_t bytesTransferred, boost::asio::const_buffer buffer)
{
	errlog << "UDP: writeHandler() ec: " << ec;
	errlog << "UDP: writeHandler() sent " << bytesTransferred;
	mPool.free(const_cast<char*>(boost::asio::buffer_cast<const char*>(buffer)));
	read();
}

} // namespace Network
} // namespace BFG
