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

#include <boost/mpl/list.hpp>

#include <Core/ClockUtils.h>
#include <Network/Packet.h>
#include <Network/Tcp.h>
#include <Network/Udp.h>


#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(IPacketTestSuite)

using namespace boost;
using namespace BFG::Network;

const BFG::u8 MAGIC_DEBUG_VALUE   = 0xCC;
const std::size_t BIG_BUFFER_SIZE = 8192;

typedef boost::mpl::list<Tcp, Udp> AllProtocolsT;
typedef array<BFG::u8, BIG_BUFFER_SIZE> BigBufferT;

Segment createTestSegment(const char* data)
{
	Segment segment;
	segment.appEventId = 12224;
	segment.destinationId = 1234567L;
	segment.senderId = 7654321L;
	segment.dataSize = strlen(data);
	return segment;
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketContainsNoData, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a very small packet
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	// It must not yet contain any data
	BOOST_REQUIRE(!iPacket.containsData());
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketContainsSegmentOnly, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a very small packet
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	Segment s = createTestSegment("");
	BOOST_REQUIRE( iPacket.add(s, "") );

	// It must now (officially) contain data (with zero size)
	BOOST_CHECK_EQUAL(s.dataSize, 0);
	BOOST_REQUIRE(iPacket.containsData());
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketAddingDataPayload, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	DataPayload dp(12345, 11111111L, 22222222L, 0, CharArray512T());
	iPacket.add(dp);
	
	BOOST_REQUIRE(iPacket.containsData());
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketContains2SegmentsWithData, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a very small packet
	BigBufferT bigBuffer;
	bigBuffer.fill(MAGIC_DEBUG_VALUE);
	
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	const char* data1 = "C++";
	const char* data2 = "Rocks";
	Segment s1 = createTestSegment(data1);
	Segment s2 = createTestSegment(data2);
	
	BOOST_REQUIRE( iPacket.add(s1, data1) );
	BOOST_REQUIRE( iPacket.add(s2, data2) );

	// It must contain data
	BOOST_CHECK_GT(s1.dataSize, 0);
	BOOST_CHECK_GT(s2.dataSize, 0);
	BOOST_REQUIRE(iPacket.containsData());
	
	// Since the same buffer was used the whole time, pointers should still
	// be the same.
	const BFG::u8* p1 = asio::buffer_cast<const BFG::u8*>(iPacket.full());
	const BFG::u8* p2 = bigBuffer.data();
	BOOST_CHECK_EQUAL(p1, p2);

	// Check sizes
	std::size_t expectedSize = ProtocolT::headerSize()
		+ sizeof(Segment)
		+ sizeof(Segment)
		+ strlen(data1)
		+ strlen(data2);
	
	BOOST_CHECK_EQUAL(iPacket.size(), expectedSize);

	// Does it really contain our data?
	BigBufferT::iterator begin = bigBuffer.begin() + ProtocolT::headerSize();
	bool data1Found = std::string(begin, bigBuffer.end()).find(data1) != std::string::npos;
	bool data2Found = std::string(begin, bigBuffer.end()).find(data2) != std::string::npos;

	BOOST_REQUIRE(data1Found);
	BOOST_REQUIRE(data2Found);
	
	// Check if there're any bytes which have been left over
	BigBufferT::iterator packetEnd = bigBuffer.begin() + expectedSize;
	std::size_t magicDebugValues = std::count(bigBuffer.begin(), packetEnd, MAGIC_DEBUG_VALUE);
	BOOST_CHECK_EQUAL(magicDebugValues, 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketAddToAlreadyStuffedPacket, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a very small packet
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	std::string data1(1000, 'A');
	std::string data2(1000, 'B');
	Segment s1 = createTestSegment(data1.c_str());
	Segment s2 = createTestSegment(data2.c_str());

	// First should be fine (even with header)
	BOOST_REQUIRE( iPacket.add(s1, data1.c_str()) );
	
	// But the second must not fit into this packet since its size
	// would exceed the buffer size.
	BOOST_REQUIRE(!iPacket.add(s2, data2.c_str()) );

	// It must contain data
	BOOST_REQUIRE(iPacket.containsData());
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketContainsData, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a very small packet
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	// Put some data in it
	const char* data = "Hallo?";
	Segment s = createTestSegment(data);
	BOOST_REQUIRE( iPacket.add(s, data) );

	// It must now contain data
	BOOST_REQUIRE(iPacket.containsData());
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketBufferTooSmall, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a packet which is a bit to small to contain
	// anything useful in it. We should get an exception.
	array<BFG::u8, 500> a500;
	BOOST_CHECK_THROW
	(
		IPacket<ProtocolT>(asio::buffer(a500), HeaderFactoryT()),
		std::length_error
	);
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketBufferFit, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a packet which should right fit into the
	// provided buffer.
	std::vector<BFG::u8> v(ProtocolT::MAX_PACKET_SIZE_BYTES);

	BOOST_CHECK_NO_THROW
	(
		IPacket<ProtocolT>(asio::buffer(v), HeaderFactoryT())
	);
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketNewBuffer, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	// Let's try to create a very small packet
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	// with some data
	const char* data = "ABCDEFGHIJKLMNOPQRSTUVWXY1234567890";
	Segment s = createTestSegment(data);
	BOOST_REQUIRE( iPacket.add(s, data) );

	// Now let's reuse the packet and create a new buffer
	BigBufferT bigBuffer2;
	bigBuffer2.fill(MAGIC_DEBUG_VALUE);
	
	iPacket.clear(asio::buffer(bigBuffer2));

	// Since its a new buffer it must not contain data.
	BOOST_CHECK_LE(iPacket.size(), ProtocolT::headerSize());

	// Ensure that there's nothing left over from the old packet.
	std::size_t magicDebugValues = std::count(bigBuffer2.begin(), bigBuffer2.end(), MAGIC_DEBUG_VALUE);
	BOOST_CHECK_EQUAL(magicDebugValues, bigBuffer2.size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE (TestIPacketSizeEqualsFullSize, ProtocolT, AllProtocolsT)
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	BigBufferT bigBuffer;
	IPacket<ProtocolT> iPacket(asio::buffer(bigBuffer), HeaderFactoryT());
	
	// with some data
	const char* data = "ABCDEFGHIJKLMNOPQRSTUVWXY1234567890";
	Segment s = createTestSegment(data);
	BOOST_REQUIRE( iPacket.add(s, data) );

	BOOST_REQUIRE_EQUAL(asio::buffer_size(iPacket.full()), iPacket.size());
}

BOOST_AUTO_TEST_SUITE_END()
