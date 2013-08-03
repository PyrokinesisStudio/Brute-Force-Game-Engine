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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Base/ResolveDns.h>
#include <Base/ShowException.h>

#include <Core/Path.h>
#include <Core/Types.h>
#include <Core/GameHandle.h>
#include <Core/CharArray.h>
#include <Core/v3.h>

#include <Event/Event.h>

#include <Network/Network.h>

#define TESTID 54321
#define EVENT_WAIT_TIME 5 // milliseconds
#define NUMBER_OF_EVENTS 10000

using namespace BFG;

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, BFG::v3& v, const unsigned int /*version*/)
{
	ar & v.x;
	ar & v.y;
	ar & v.z;
}

} // namespace serialization
} // namespace boost

template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

void networkEventTest(Event::Lane& lane)
{
	dbglog << "Waiting 5 seconds before starting sending NetworkEvents";
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	for (u32 i = 0; i < NUMBER_OF_EVENTS; ++i)
	{
		std::stringstream ss;
		boost::archive::text_oarchive oa(ss);

		BFG::v3 randomVec((f32)rand()/(f32)RAND_MAX, (f32)rand()/(f32)RAND_MAX, (f32)rand()/(f32)RAND_MAX);
		oa << randomVec;

		CharArray512T ca512 = stringToArray<512>(ss.str());

		Network::DataPayload payload(TESTID, generateHandle(), generateHandle(), ss.str().size(), ca512);

		lane.emit(ID::NE_SEND, payload);
		boost::this_thread::sleep(boost::posix_time::milliseconds(EVENT_WAIT_TIME));
	}
}

int main( int argc, const char* argv[] ) try
{
	bool server = false;
	if  (argc == 2)
		server = true;
	else if (argc == 3)
		server = false;
	else
	{
		std::cerr << "For Server use: bfgNetworkTest <Port>\nFor Client use: bfgNetworkTest <IP> <Port>\n";
		BFG::Base::pause();
		return 0;
	}

	Event::Synchronizer synchronizer;
	Event::Lane lane(synchronizer, 100);

	if (server)
	{
		u16 port = 0;

		if (!from_string(port, argv[1], std::dec))
		{
			std::cerr << "Port not a number: " << argv[1] << std::endl;
			BFG::Base::pause();
			return 0;
		}

		Path p;
		Base::Logger::Init(Base::Logger::SL_DEBUG, p.Get(ID::P_LOGS) + "/NetworkServerTest.log");

		dbglog << "Starting as Server";


		int mode(BFG_SERVER); // casting problem
		lane.addEntry<Network::Main>(mode);

		synchronizer.startEntries();
		
		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		lane.emit(ID::NE_LISTEN, port);

		Base::pause();
		dbglog << "Good bye";
	}
	else
	{
		std::string ip(argv[1]);
		std::string port(argv[2]);

		Path p;
		Base::Logger::Init(Base::Logger::SL_DEBUG, p.Get(ID::P_LOGS) + "/NetworkClientTest.log");

		dbglog << "Starting as Client";

		int mode(BFG_CLIENT);
		lane.addEntry<Network::Main>(mode);
		synchronizer.startEntries();

		boost::this_thread::sleep(boost::posix_time::milliseconds(500));

		Network::EndpointT payload = make_tuple(stringToArray<128>(ip), stringToArray<128>(port));

		lane.emit(ID::NE_CONNECT, payload);
		networkEventTest(lane);

		Base::pause();
		dbglog << "Good bye";
	}
		
	// Give EventSystem some time to stop all loops
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
