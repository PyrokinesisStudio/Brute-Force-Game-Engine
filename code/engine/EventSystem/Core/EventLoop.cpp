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

#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Core/EventManager.h>

EventLoop::EventLoop(bool notifyLoopEventListener,
                     EventSystem::ThreadingPolicy* tp,
                     EventSystem::CommunicationPolicy* cp,
                     EventSystem::ExceptionPolicy* ep) :
EventProcessor(ep),
mFrontPool(new BaseEventPool()),
mBackPool(new BaseEventPool()),
mThreadingPolicy(tp),
mCommunicationPolicy(cp),
mExceptionPolicy(ep),
mShouldExit(false),
mMultiLoop(notifyLoopEventListener)
{}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
	mThreadingPolicy->start(this);
}

void EventLoop::stop()
{
	setExitFlag();
}

void EventLoop::entry()
{
	try
	{
		mCommunicationPolicy->init();
		if ( !hasEntryPoints() )
		{
			throw std::logic_error("EventLoop: This loop has no EntryPoints!");
		}

		// Run synchronously
		callEntryPoints(this);
		
		while(!mShouldExit)
			doLoop();

	//	mCommunicationPolicy->deinit();
		mThreadingPolicy->stop();
	}
	catch (const std::exception& ex)
	{
		mExceptionPolicy->onStdException(ex);
	}
	catch (...)
	{
		mExceptionPolicy->onUnknownException();
	}
}

long EventLoop::doLoop()
{
	if (mMultiLoop)
	{
		// Notify the LoopEvent listeners
		LoopEvent ev(this, 0);
		mLoopEventReceivers.call(&ev);
	}

	long eventCount = 0;
	BaseEventPool* incoming = 0;

	// While Events are stored
	do 
	{
		while (mFrontPool->size() > 0)
		{
			// 1. Switch Buffers
			lock();
			mFrontPool.swap(mBackPool);
			unlock();

			// 2. Process BackPool
			eventCount += processEventsCount(mBackPool.get(), 0);

			// 2.1. Publish BackPool
			mCommunicationPolicy->publishPool(mBackPool.get());

			mBackPool->clear();
		}	

		if (incoming && incoming->size() > 0)
		{
			eventCount += processEventsCount(incoming, 0);
			EventManager::getInstance()->freePool(incoming);
		}
		incoming = mCommunicationPolicy->receivePool();
	} 
	while(incoming);

	return eventCount;
}
