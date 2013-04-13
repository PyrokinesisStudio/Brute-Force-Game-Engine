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

#ifndef BFG_NETWORK_PACKET_H
#define BFG_NETWORK_PACKET_H

#include <cstring>

#include <boost/asio/buffer.hpp>

#include <Network/DataPayload.h>
#include <Network/Defs.h>
#include <Network/Event.h>
#include <Network/Segment.h>

namespace BFG {
namespace Network {

template <typename ProtocolT>
class IPacket
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	typedef typename ProtocolT::HeaderT        HeaderT;

public:
	//! Constructing before write()
	IPacket(boost::asio::mutable_buffer buffer, const HeaderFactoryT& factory) :
	mBuffer(buffer),
	mOffset(headerSize()),
	mHeaderFactory(factory)
	{
		throwIfBufferTooSmall();
	}
	
	bool add(Segment& segment, const char* data)
	{
		if (!maySwallowSegmentAndData(segment))
			return false;

		boost::asio::buffer_copy
		(
			mBuffer + mOffset,
			boost::asio::buffer(&segment, sizeof(Segment))
		);
		mOffset += sizeof(Segment);

		boost::asio::buffer_copy
		(
			mBuffer + mOffset,
			boost::asio::buffer(data, segment.dataSize)
		);
		mOffset += segment.dataSize;

		return true;
	}
	
	bool add(const DataPayload& payload)
	{
		Segment s;
		s.appEventId = payload.mAppEventId;
		s.destinationId = payload.mAppDestination;
		s.senderId = payload.mAppSender;
		s.dataSize = payload.mAppDataLen;
		return add(s, payload.mAppData.data());
	}

	//! Returns a raw byte pointer to the packet's content.
	//! The packet header get's added at this point.
	boost::asio::const_buffer full()
	{
		makeHeader();
		return boost::asio::buffer(mBuffer, size());
	}
	
	std::size_t size() const
	{
		return mOffset;
	}

	//! Makes the packet reusable. A new buffer must be provided, though.
	void clear(boost::asio::mutable_buffer buffer)
	{
		mBuffer = buffer;
		mOffset = headerSize();
	}
	
	//! Checks if the packet contains at least one segment
	bool containsData() const
	{
		const std::size_t minimumAmount = headerSize() + sizeof(Segment);

		return this->size() >= minimumAmount;
	}
	
private:
	static std::size_t headerSize()
	{
		return ProtocolT::headerSize();
	}
	
	boost::asio::const_buffer dataPart() const
	{
		return boost::asio::buffer
		(
			mBuffer + ProtocolT::headerSize(),
			mOffset - ProtocolT::headerSize()
		);
	}
	
	void makeHeader()
	{
		HeaderT header = mHeaderFactory.createFrom(dataPart());
		
		// Integrate header into packet
		typename HeaderT::SerializationT output;
		header.serialize(output);
		boost::asio::buffer_copy(mBuffer, boost::asio::buffer(output));
	}
	
	bool maySwallowSegmentAndData(const Segment& segment) const
	{
		std::size_t requiredSize = segment.dataSize + sizeof(Segment);
		std::size_t sizeLeft = ProtocolT::MAX_PACKET_SIZE_BYTES - mOffset;

		bool hasEnoughSpace = requiredSize <= sizeLeft;
		return hasEnoughSpace;
	}

	void throwIfBufferTooSmall() const
	{
		if (boost::asio::buffer_size(mBuffer) < ProtocolT::MAX_PACKET_SIZE_BYTES)
		{
			std::stringstream ss;
			ss << "Network::IPacket: Buffer size ("
			   << boost::asio::buffer_size(mBuffer)
			   << " bytes) too small. At least "
			   << ProtocolT::MAX_PACKET_SIZE_BYTES
			   << " bytes are required.";
			throw std::length_error(ss.str());
		}
	}

	boost::asio::mutable_buffer mBuffer;
	u16 mOffset;
	
	const HeaderFactoryT& mHeaderFactory;
};

template <typename ProtocolT>
class OPacket
{
public:
	//! Construction must happen after headers have been parsed
	//! \param buffer Contains the data part only
	OPacket(boost::asio::const_buffer buffer) :
	mBuffer(buffer),
	mOffset(0)
	{
		throwIfBufferTooBig();
	}
	
	bool hasNextPayload() const
	{
		return mOffset + sizeof(Segment) <= boost::asio::buffer_size(mBuffer);
	}
	
	DataPayload nextPayload(PayloadFactory& payloadFactory)
	{
		if (!hasNextPayload())
		{
			throw std::range_error(
				"Network::OPacket: No data left within this"
				" packet. Please Check with hasNextPayload()."
			);
		}
		
		Segment s;
		boost::asio::buffer_copy
		(
			boost::asio::buffer(&s, sizeof(Segment)),
			mBuffer + mOffset
		);
		mOffset += sizeof(Segment);

		CharArray512T ca;
		boost::asio::buffer_copy
		(
			boost::asio::buffer(ca),
			mBuffer + mOffset,
			s.dataSize
		);
		mOffset += s.dataSize;

		DataPayload payload = payloadFactory.create(s, ca);
		return payload;
	}
	
private:
	void throwIfBufferTooBig() const
	{
		if (boost::asio::buffer_size(mBuffer) > ProtocolT::MAX_PACKET_SIZE_BYTES)
		{
			std::stringstream ss;
			ss << "Network::IPacket: Buffer size ("
			   << boost::asio::buffer_size(mBuffer)
			   << " bytes) too big to be a real packet (which should be "
			   << ProtocolT::MAX_PACKET_SIZE_BYTES
			   << " bytes at maximum).";
			throw std::length_error(ss.str());
		}
	}
	
	boost::asio::const_buffer mBuffer;
	
	std::size_t mOffset;
};

} // namespace Network
} // namespace BFG

#endif
