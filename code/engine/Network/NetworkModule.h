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

//! A NetworkModule is used to communicate over the network.
//! The data can be send using one of the SEND events.
//! Received data is send via the NE_RECEIVED event.
template <typename ProtocolT>
class NetworkModule : public Emitter, public boost::enable_shared_from_this<NetworkModule<ProtocolT> >
{
public:
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
	mPool(ProtocolT::MAX_BYTE_RATE),
	mSendPacket(createBuffer(mPool), mHeaderFactory)
	{
		mFlushTimer.reset(new boost::asio::deadline_timer(service));
	}

	//! \brief Destructor
	~NetworkModule()
	{
		mFlushTimer.reset();

		dbglog << "NetworkModule::~NetworkModule (" << this << ")";
		loop()->disconnect(ProtocolT::EVENT_ID_FOR_SENDING, this);
	}
	
	//! \brief The connection is ready to receive data
	void startReading()
	{
		dbglog << "NetworkModule::startReading";
		setFlushTimer(FLUSH_WAIT_TIME);

		// TODO: Document why both connects are necessary
		loop()->connect(ProtocolT::EVENT_ID_FOR_SENDING, this, &NetworkModule<ProtocolT>::dataPacketEventHandler);

		if (mPeerId)
			loop()->connect(ProtocolT::EVENT_ID_FOR_SENDING, this, &NetworkModule<ProtocolT>::dataPacketEventHandler, mPeerId);

		// TODO: Call from Client or Server
//		setTcpDelay(false);
		
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
	
	//! \brief Handler for the writing of data
	//! \param[in] ec Error code of boost::asio
	//! \param[in] bytesTransferred size of the data written
	void writeHandler(const boost::system::error_code &ec,
	                  std::size_t bytesTransferred,
	                  boost::asio::const_buffer buffer)
	{
		dbglog << "NetworkModule::writeHandler: " << bytesTransferred << " Bytes written";
		if (ec)
		{
			printErrorCode(ec, "writeHandler");
		}
		mPool.free(const_cast<char*>(boost::asio::buffer_cast<const char*>(buffer)));
	} 

	//! \brief Logs an error_code
	//! \param[in] ec Error code to log
	//! \param[in] method Name of the method that received the error
	void printErrorCode(const boost::system::error_code &ec, const std::string& method)
	{
		warnlog << "This (" << this << ") " << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
	}

	typedef typename ProtocolT::HeaderT HeaderT;
	typedef typename HeaderT::SerializationT HeaderSerializationT;
	
	const PeerIdT mPeerId;

	boost::shared_ptr<Clock::StopWatch> mLocalTime;

private:
	//! \brief Reset the time for the next automatic flush
	//! \param[in] waitTime_ms Time in milliseconds to the next flush
	void setFlushTimer(const long& waitTime_ms)
	{
		if (waitTime_ms == 0)
			return;

		mFlushTimer->expires_from_now(boost::posix_time::milliseconds(waitTime_ms));
		mFlushTimer->async_wait(boost::bind(&NetworkModule<ProtocolT>::flushTimerHandler, this->shared_from_this(), _1));
	}

	//! \brief Handler for the flush timer
	//! \param[in] ec Error code of boost asio
	void flushTimerHandler(const boost::system::error_code &ec)
	{
		if (!ec)
		{
			flush();
			setFlushTimer(FLUSH_WAIT_TIME);
		}
		else if (ec.value() == boost::asio::error::operation_aborted)
		{
			warnlog << "NetworkModule: Flush timer was cancelled! (PeerID: " << mPeerId << ")";
		}
		else
		{
			printErrorCode(ec, "flushTimerHandler");
		}
	}

	//! \brief Handler for DataPacketEvents
	//! \param[in] e The DataPacketEvent to distribute
	void dataPacketEventHandler(DataPacketEvent* e)
	{
		switch(e->id())
		{
		case ProtocolT::EVENT_ID_FOR_SENDING:
			onSend(e->getData());
			break;
		default:
			warnlog << "NetworkModule: Can't handle event with ID: "
			        << e->id();
			break;
		}
	}

	//! \brief Start asynchronous reading from the connected network module
	virtual void read() = 0;
	
	//! \brief Perform an asynchronous write of data to the connected network module
	//! \param[in] packet data to write over the net
	//! \param[in] size Size of the data set
	virtual void write(boost::asio::const_buffer packet, std::size_t size) = 0;
	
	//! \brief Received data from the net is packed as a corresponding event 
	virtual void onReceive(OPacket<ProtocolT>& oPacket, PeerIdT peerId) = 0;

	boost::pool<> mPool;

	typename ProtocolT::HeaderFactoryT mHeaderFactory;
	
	IPacket<ProtocolT> mSendPacket;

	boost::shared_ptr<boost::asio::deadline_timer> mFlushTimer;
	boost::mutex                                   mFlushMutex;
};

} // namespace Network
} // namespace BFG

#endif
