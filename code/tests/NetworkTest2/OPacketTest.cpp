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

#include <Network/Packet.h>
#include <Network/Rtt.h>
#include <Network/Tcp.h>
#include <Network/Udp.h>

#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(OPacketTestSuite)

using namespace boost;
using namespace BFG::Network;
using namespace BFG::Clock;

const BFG::u8 MAGIC_DEBUG_VALUE   = 0xCC;
const std::size_t BIG_BUFFER_SIZE = 8192;

typedef boost::mpl::list<Tcp, Udp> AllProtocolsT;
typedef array<BFG::u8, BIG_BUFFER_SIZE> BigBufferT;

BOOST_AUTO_TEST_CASE_TEMPLATE (TestOPacketIncomingBufferSize, ProtocolT, AllProtocolsT)
{
	// OPacket works with a buffer which is exactly as long as the length of
	// the size received data. That is, if we received 423 bytes of data,
	// the buffer should be of the same size.
	
	array<BFG::u8, ProtocolT::MAX_PACKET_SIZE_BYTES + 100>
		bufferExcedingMaximumPacketSize;

	// A size of an incoming buffer (like 100.000 bytes) which exceeds the
	// maximum protocol packet size (like 1.400 bytes) thus is an indicator
	// for a programming error. Also, invalid packet sizes must be handled
	// before OPacket begins its work. OPacket is be able to detected such
	// an error.
	BOOST_CHECK_THROW
	(
		OPacket<ProtocolT>(asio::buffer(bufferExcedingMaximumPacketSize)),
		std::length_error
	);
	
	// Other (allowed) cases:
	
	array<BFG::u8, ProtocolT::MAX_PACKET_SIZE_BYTES>
		bufferWithMaximumPacketSize;
	array<BFG::u8, ProtocolT::MAX_PACKET_SIZE_BYTES - 10>
		bufferSmallerThanMaximumPacketSize;
	
	BOOST_CHECK_NO_THROW
	(
		OPacket<ProtocolT>(asio::buffer(bufferWithMaximumPacketSize))
	);
	BOOST_CHECK_NO_THROW
	(
		OPacket<ProtocolT>(asio::buffer(bufferSmallerThanMaximumPacketSize))
	);

	
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketOPacket, ProtocolT, AllProtocolsT)
{
	// First create some test data
	const char* data1 = "Test";
	const char* data2 = "Blah";

	Segment s1;
	s1.appEventId = 12345;
	s1.dataSize = strlen(data1);
	s1.destinationId = 999999999L;
	s1.senderId = 111111111L;
	
	Segment s2;
	s2.appEventId = 67890;
	s2.dataSize = strlen(data2);
	s2.destinationId = 888888888L;
	s2.senderId = 222222222L;

	// Put everything into an IPacket
	BigBufferT iBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(iBuffer), typename ProtocolT::HeaderFactoryT());
	iPacket.add(s1, data1);
	iPacket.add(s2, data2);
	
	// Now cut the header part off and pretend we received the data-only part
	asio::const_buffer buffer = iPacket.full();
	asio::const_buffer received = asio::buffer(buffer + ProtocolT::headerSize());
	
	// Finally we might test if we get the same data out of it
	OPacket<ProtocolT> oPacket(received);
	
	// There must be data in it
	BOOST_REQUIRE_EQUAL(oPacket.hasNextPayload(), true);
	
	// Some timely preparations for extraction
	const BFG::s32 fakeTimestampOffset = 2222;
	boost::shared_ptr<StopWatch> fakeLocaltime(new StopWatch(milliSecond));
	fakeLocaltime->start();
	Rtt<BFG::s32, 10> fakeRtt;
	PayloadFactory pf(fakeTimestampOffset, fakeLocaltime, fakeRtt);

	// Try to get it
	DataPayload dp1 = oPacket.nextPayload(pf);

	// There must still be data in it
	BOOST_REQUIRE_EQUAL(oPacket.hasNextPayload(), true);

	// Try to get it
	DataPayload dp2 = oPacket.nextPayload(pf);

	// Now there must not be any data left within
	BOOST_REQUIRE_EQUAL(oPacket.hasNextPayload(), false);

	// Next attempt to fetch data must fail
	BOOST_CHECK_THROW
	(
		oPacket.nextPayload(pf),
		std::range_error
	);

	// Check data
	BOOST_REQUIRE_EQUAL(dp1.mAppEventId, s1.appEventId);
	BOOST_REQUIRE_EQUAL(dp1.mAppDataLen, s1.dataSize);
	BOOST_REQUIRE_EQUAL(dp1.mAppDestination, s1.destinationId);
	BOOST_REQUIRE_EQUAL(dp1.mAppSender, s1.senderId);

	// Check data
	BOOST_REQUIRE_EQUAL(dp2.mAppEventId, s2.appEventId);
	BOOST_REQUIRE_EQUAL(dp2.mAppDataLen, s2.dataSize);
	BOOST_REQUIRE_EQUAL(dp2.mAppDestination, s2.destinationId);
	BOOST_REQUIRE_EQUAL(dp2.mAppSender, s2.senderId);
}

BOOST_AUTO_TEST_SUITE_END()
