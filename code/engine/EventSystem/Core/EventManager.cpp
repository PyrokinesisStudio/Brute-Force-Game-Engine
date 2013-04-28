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

#include <Base/Cpp.h>
#include <Base/Logger.h>
#include <Base/NameCurrentThread.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Core/EventManager.h>

EventManager* EventManager::getInstance()
{
	return BFG::Base::CSingletonT<EventManager>::instance();
}

EventManager::EventManager() :
mThreadCount(0),
mExitFlag(false),
mEventIdDynamicStartValue(30000)
{
	mThread = boost::thread(&EventManager::runThread, this);
}

EventManager::~EventManager(void)
{
	mExitFlag = true;
	// Wait till thread is finished, then do cleanup
	mThread.join();
	// Do real cleanup here
	while ( !mPoolQueue.empty() )
	{
		delete mPoolQueue.front();
		mPoolQueue.pop_front();
	}
}

void EventManager::runThread()
{
	BFG::nameCurrentThread("EventManager");

	size_t operationCounter = 0;
	while(!mExitFlag)
	{
		// Do normal operation
		if (!doOperate())
		{
			++operationCounter;
			if (operationCounter < 10)
				boost::thread::yield();
			else
			{
				boost::thread::sleep
				(
					boost::get_system_time() + boost::posix_time::milliseconds(1)
				);
			}
		}
		else
		{
			operationCounter = 0; 
		}
	}
}

long EventManager::registerEventLoop(EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);
	mEventChannelList[mThreadCount] = ioChannel;
	long wRet = ++mThreadCount;
	mEventChannelList[mThreadCount] = NULL;
	return wRet;
}

void EventManager::unregisterEventLoop(EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);

	int channel_id = getChannelId(ioChannel, false);
	if (channel_id == -1)
		return;

	if (channel_id == (mThreadCount - 1))
	{
		mEventChannelList[channel_id] = NULL;
	}
	else
	{
		mEventChannelList[channel_id] = mEventChannelList[mThreadCount - 1];
		mEventChannelList[mThreadCount - 1] = NULL;
	}	
	--mThreadCount;
}

void EventManager::subscribeEvent(long eventId, EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mExecutionListMutex);

	EventChannelMaskMap::iterator it = mExecutionList.find (eventId);
	long mask = 0;
	if ( it != mExecutionList.end() )
	{	
		mask = (*it).second;
	}	
	int id = getChannelId(ioChannel);
	mExecutionList[eventId] = mask | (1 << id);
}

void EventManager::unsubscribeEvent(long eventId, EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mExecutionListMutex);

	EventChannelMaskMap::iterator it = mExecutionList.find(eventId);
	long mask = 0;
	if (it != mExecutionList.end())
	{	
		mask = (*it).second;
	}	
	int id = getChannelId(ioChannel);

	if (mask && (1 << id))
	{
		mExecutionList[eventId] = mask - (1 << id);
	}

}

void EventManager::unsubscribeAll(EventChannel* ioChannel)
{
	// Takes a very long time
	boost::mutex::scoped_lock scoped_lock(mExecutionListMutex);

	EventChannelMaskMap::iterator it = mExecutionList.begin();

	int id = getChannelId(ioChannel);
	if (id == -1)
		return;
	
	long channelmask = (1 << id);
	while(it != mExecutionList.end())
	{
		if ((*it).second & channelmask)
		{
			long mask = (*it).second;
			mExecutionList[(*it).first] = (mask - channelmask);
		}
		++it;
	}

}

//!
//! After many thoughts and considerations, this function will be implemented using the Read-Only-Doctrine.
//! All Events have to be considered Read-Only, because there will be many readers.
//!
//! \return true if caller has spend cpu-time

bool EventManager::doOperate()
{
	//! \todo rename InternalMutex to OperationMutex or something
	//! InternalMutex is only for DoOperate function ?
	boost::try_mutex::scoped_try_lock internal_lock(mInternalMutex); 
	if (!internal_lock.owns_lock())
	{
		return false;
	}

	// Aquire all necessary Locks
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);

	bool didWork = false;
	for (size_t i = 0; i < mThreadCount; ++i)
	{
		BaseEventPool* pool;
		while ((pool = mEventChannelList[i]->getPublishedEventPool()))
		{
			didWork = true;

			// transmit to all other threads
			for (size_t j = 0; j < mThreadCount; ++j)
			{
				if (j != i)
				{
					BaseEventPool* poolClone = requestPool();
					pool->copyTo(poolClone);
					mEventChannelList[j]->receiveEventPool(poolClone);
				}
			}

			// we don't need this pool anymore
			freePool(pool);
		}
	}
	return didWork;
}

long EventManager::getDynamicEventId(const std::string& eventIdentifier)
{
	long wRet = 0;
	boost::mutex::scoped_lock scoped_lock(mDynamicEventIdMapMutex);

	EventIdBinding::iterator it = mDynamicEventIdMap.find(eventIdentifier);
	if (it == mDynamicEventIdMap.end())
	{
		mDynamicEventIdMap[eventIdentifier] = mEventIdDynamicStartValue;
		wRet = mEventIdDynamicStartValue;
		++mEventIdDynamicStartValue;
	}
	else
	{
		wRet = (*it).second;
	}

	return wRet;
}

int EventManager::getChannelId(EventChannel* ioChannel, bool should_lock) 
{ 
	if (should_lock)
	{
		//! \bug This does probably nothing of use as it goes immediately out of scope.
		boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);
	}

	int wRet = 0; 
	while(mEventChannelList[wRet] != NULL) 
	{ 
		if (mEventChannelList[wRet] == ioChannel)
		{
			return wRet;
		}
		++wRet;
	}
	return -1;
}

// Pool Operations

BaseEventPool* EventManager::requestPool()
{
	BaseEventPool* wRet = NULL;

	boost::recursive_mutex::scoped_lock scoped_lock(mEventPoolMutex);

	if (mPoolQueue.empty())
	{
		wRet = new BaseEventPool();
		//dbglog << "EM(" << this << "): New Pool created";
	}
	else
	{
		wRet = mPoolQueue.front();
		mPoolQueue.pop_front();
		//dbglog << "EM(" << this << "): Pool from queue. New queuesize: " << mPoolQueue.size();
	}
	return wRet;
}

void EventManager::freePool(BaseEventPool* pool)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mEventPoolMutex);
	
	pool->clear();
	mPoolQueue.push_back(pool);
	//dbglog << "EM(" << this << "): Pool freed. New queuesize: " << mPoolQueue.size();
}

