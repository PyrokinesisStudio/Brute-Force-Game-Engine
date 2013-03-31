/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

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

#ifndef __BFG_NETWORKMODULE_H__
#define __BFG_NETWORKMODULE_H__

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/pool/pool.hpp>

#include <Core/ClockUtils.h>
#include <Core/Types.h>
#include <EventSystem/Emitter.h>

#include <Network/CreateBuffer.h>
#include <Network/Defs.h>
#include <Network/Event_fwd.h>
#include <Network/Packet.h>

class EventLoop;

namespace BFG {
namespace Network {

using namespace boost::asio::ip;
using namespace boost::system;

//! A NetworkModule is used to communicate over the network. The data can be send using the NE_SEND event. 
//! Received data is send via the NE_RECEIVED event.
template <typename ProtocolT>
class NetworkModule : public Emitter, public boost::enable_shared_from_this<NetworkModule<ProtocolT> >
{
public:
	typedef tcp::socket SocketT;
	
	//! \brief Constructor
	//! \param[in] loop Eventloop the NetworkModule is connected to 
	//! \param[in] service Asio service for the network connection
	//! \param[in] peerId ID of the module for identification over the network
	//! \param[in] localTime The local time of this module
	NetworkModule(EventLoop* loop_,
	              boost::asio::io_service& service,
	              PeerIdT peerId,
	              boost::shared_ptr<Clock::StopWatch> localTime) :
	BFG::Emitter(loop_),
	mPeerId(peerId),
	mLocalTime(localTime),
	mPool(PACKET_MTU*2),
	mSendPacket(createBuffer(mPool), mHeaderFactory)
	{
		mSocket.reset(new SocketT(service));
		mTimer.reset(new boost::asio::deadline_timer(service));
	}

	//! \brief Destructor
	~NetworkModule()
	{
		mTimer.reset();

		dbglog << "NetworkModule::~NetworkModule (" << this << ")";
		loop()->disconnect(ID::NE_SEND, this);

		mSocket.reset();
	}
	
	//! \brief Returns the socket of the connection
	//! \return tcp::socket of the connection
	boost::shared_ptr<SocketT> socket()
	{
		return mSocket;
	}

	//! \brief The connection is ready to receive data
	void startReading()
	{
		dbglog << "NetworkModule::startReading";
		setFlushTimer(FLUSH_WAIT_TIME);

		loop()->connect(ID::NE_SEND, this, &NetworkModule<ProtocolT>::dataPacketEventHandler);
		if (mPeerId)
			loop()->connect(ID::NE_SEND, this, &NetworkModule<ProtocolT>::dataPacketEventHandler, mPeerId);

		setTcpDelay(false);
		
		read();
	}
	
protected:
	//! \brief Sending data to the connected network module
	//! With this method the data is queued and flushes automatically every few milliseconds 
	//! or if the queue is full
	//! \param[in] payload Data to send
	void onSend(DataPayload& payload)
	{
		dbglog << "NetworkModule::onSend(): Current Time: " << mLocalTime->stop();

		mFlushMutex.lock();
		dbglog << "NetworkModule::onSend(): "
		       << payload.mAppDataLen + sizeof(Segment)
		       << " (" << sizeof(Segment) << " + " << payload.mAppDataLen << ")";

		bool added = mSendPacket.add(payload);
		mFlushMutex.unlock();
		
		if (!added)
		{
			flush();
			onSend(payload);
		}
	}
	
	//! \brief Flushes the data queue to send its data to the connected network module
	void flush()
	{
		boost::mutex::scoped_lock scopedLock(mFlushMutex);

		// Nothing to write?
		if (!mSendPacket.containsData())
			return;

		dbglog << "NetworkModule -> Flushing: " << mSendPacket.size() << " bytes";

		write
		(
			mSendPacket.full(),
			mSendPacket.size()
		);

		// Cleanup
		mSendPacket.clear(createBuffer(mPool));
	}
	
	// TODO: Make const
	PeerIdT mPeerId;

	boost::shared_ptr<Clock::StopWatch> mLocalTime;

private:
	//! \brief Reset the time for the next automatic flush
	//! \param[in] waitTime_ms Time in milliseconds to the next flush
	void setFlushTimer(const long& waitTime_ms);

	//! \brief Enable or disable Nagle's Algorithm
	//! Enabling this could cause a delay in latency
	//! \param[in] on Enable delay or not
	void setTcpDelay(bool on);
	
	//! \brief Perform an asynchronous write of data to the connected network module
	//! \param[in] packet data to write over the net
	//! \param[in] size Size of the data set
	void write(boost::asio::const_buffer packet, std::size_t size);

	//! \brief Start asynchronous reading from the connected network module
	void read();

	//! \brief Handler for the flush timer
	//! \param[in] ec Error code of boost asio
	void flushTimerHandler(const error_code &ec);

	//! \brief Handler for the reading of the data header
	//! \param[in] ec Error code of boost asio
	//! \param[in] bytesTransferred size of the data received
	void readHeaderHandler(const error_code &ec, std::size_t bytesTransferred);

	//! \brief Handler for the reading of the data
	//! \param[in] ec Error code of boost asio
	//! \param[in] bytesTransferred size of the data received
	//! \param[in] pacetChecksum Checksum of the data packet
	void readDataHandler(const error_code &ec, std::size_t bytesTransferred, u32 packetChecksum);

	//! \brief Handler for the writing of data
	//! \param[in] ec Error code of boost::asio
	//! \param[in] bytesTransferred size of the data written
	void writeHandler(const error_code &ec, std::size_t bytesTransferred, boost::asio::const_buffer buffer);
	
	//! \brief Handler for DataPacketEvents
	//! \param[in] e The DataPacketEvent to distribute
	void dataPacketEventHandler(DataPacketEvent* e);

	//! \brief Received data from the net is packed as a corresponding event 
	virtual void onReceive(OPacket<ProtocolT>& oPacket) = 0;
	
	//! \brief Logs an error_code
	//! \param[in] ec Error code to log
	//! \param[in] method Name of the method that received the error
	void printErrorCode(const error_code &ec, const std::string& method);

	boost::pool<> mPool;

	typename ProtocolT::HeaderFactoryT mHeaderFactory;
	IPacket<ProtocolT> mSendPacket;

	boost::array<char, PACKET_MTU> mWriteBuffer;
	boost::array<char, PACKET_MTU> mReadBuffer;

	typedef typename ProtocolT::HeaderT HeaderT;
	typedef typename HeaderT::SerializationT HeaderSerializationT;
	
	HeaderSerializationT mWriteHeaderBuffer;
	HeaderSerializationT mReadHeaderBuffer;

	boost::shared_ptr<SocketT>                     mSocket;
	boost::shared_ptr<boost::asio::deadline_timer> mTimer;
	boost::mutex                                   mFlushMutex;
};

} // namespace Network
} // namespace BFG

#include <Network/NetworkModule.cpp>

#endif
