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

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Core/CharArray.h>
#include <Network/Checksum.h>
#include <Network/Handshake.h>

#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(OPacketTestSuite)

const BFG::u8 MAGIC_DEBUG_VALUE   = 0xCC;

BOOST_AUTO_TEST_CASE (testHandshakeSerialization)
{
	const BFG::Network::PeerIdT expectedPeerId = 12345;
	const BFG::u16 expectedProtocolVersion = 11111;
	const auto expectedUuid = boost::uuids::random_generator()();

	// Create Handshake instance
	BFG::Network::Handshake hs;
	hs.mPeerId = expectedPeerId;
	hs.mProtocolVersion  = expectedProtocolVersion;
	hs.mUdpConnectionToken = expectedUuid;
	hs.mChecksum = BFG::Network::calculateHandshakeChecksum(hs);

	// Test serialization
	BFG::Network::Handshake::SerializationT buffer;
	hs.serialize(buffer);

	BFG::Network::Handshake hs2;
	hs2.deserialize(buffer);
	
	// Compare
	BOOST_CHECK_EQUAL(hs2.mPeerId, expectedPeerId);
	BOOST_CHECK_EQUAL(hs2.mProtocolVersion, expectedProtocolVersion);
	BOOST_CHECK_EQUAL(hs2.mUdpConnectionToken, expectedUuid);
	BOOST_CHECK_EQUAL(hs2.mChecksum, BFG::Network::calculateHandshakeChecksum(hs2));
}

BOOST_AUTO_TEST_SUITE_END()
