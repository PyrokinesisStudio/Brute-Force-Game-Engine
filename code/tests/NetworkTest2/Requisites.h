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

#ifndef BFG_TEST_NETWORKTEST2_REQUISITES_H
#define BFG_TEST_NETWORKTEST2_REQUISITES_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>

#include <Network/Network.h>

using namespace boost;

struct EventStatus
{
	EventStatus(){ reset(); }
	
	void reset()
	{
		gotConnected = false;
		gotDisconnected = false;
		gotReceived = false;
		gotTcpData = false;
		gotUdpData = false;
	}
	
	bool operator == (const EventStatus& rhs) const
	{
		return
			gotConnected == rhs.gotConnected &&
			gotDisconnected == rhs.gotDisconnected &&
			gotReceived == rhs.gotReceived &&
			gotTcpData == rhs.gotTcpData &&
			gotUdpData == rhs.gotUdpData;
	}
	
	bool operator != (const EventStatus& rhs) const
	{
		return !this->operator == (rhs);
	}
	
	bool gotConnected;
	bool gotDisconnected;
	bool gotReceived;
	bool gotTcpData;
	bool gotUdpData;
};

template <typename T>
void periodicWaitForEqual(const T& lhs, const T& rhs, boost::posix_time::time_duration wait)
{
	using namespace boost;
	posix_time::time_duration slept;
	const posix_time::time_duration interval(posix_time::milliseconds(100));
	while (lhs != rhs && slept < wait)
	{
		this_thread::sleep(interval);
		slept += interval;
	}
}

template <typename ApplicationT, BFG::u8 mode, BFG::GameHandle appHandle>
struct NetworkContext
{
	boost::shared_ptr<EventLoop> loop;
	boost::scoped_ptr<ApplicationT> application;
	boost::scoped_ptr<BFG::Base::IEntryPoint> ep;
	boost::shared_ptr<BFG::Emitter> emitter;
	EventStatus status;
	
	NetworkContext(const char*const Threadname, const std::string& testMsg)
	{
		loop.reset(new EventLoop(false, new EventSystem::BoostThread<>(Threadname), new EventSystem::NoCommunication()));
		ep.reset(BFG::Network::Interface::getEntryPoint(mode));
		loop->addEntryPoint(ep.get());
		application.reset(new ApplicationT(loop.get(), status, appHandle, testMsg));
		loop->run();
		emitter.reset(new BFG::Emitter(loop.get()));
	}
	
	~NetworkContext()
	{
		application.reset();
		loop->setExitFlag();
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		emitter.reset();
		ep.reset();
		loop.reset();
	}
};

#endif
