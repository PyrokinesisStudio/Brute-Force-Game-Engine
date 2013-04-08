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

#include <boost/utility.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/variant.hpp>

#include <Base/Logger.h>
#include <Base/Cpp.h>
#include <Base/Pause.h>
#include <Base/ResolveDns.h>

#include <Core/Path.h>
#include <Core/Utils.h>
#include <Core/Enums.hh>

#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Core/EventManager.h>
#include <EventSystem/Emitter.h>
#include <EventSystem/EventFactory.h>

#include <Controller/Action.h>
#include <Controller/Controller.h>
#include <Controller/ControllerEvents.h>
#include <Controller/Enums.hh>
#include <Controller/EventDumper.h>
#include <Controller/OISUtils.h>

#include <Network/Network.h>

#include <View/WindowAttributes.h>

#ifdef _WIN32
	#include "InputWindowWin32.h"
#elif defined(linux) || defined(__linux)
	#include "InputWindowX11.h"
#endif

using namespace BFG;

#undef main
#undef SDL_main

const int DEFAULT_FREQUENCY = 200;

const char PROGRAM_DESCRIPTION[] =
	"\nController Test suite\n"
	"Copyright Brute-Force Games GbR.\n"
	"\n"
	"Notes:\n"
	"\n"
	"* The small gray window (which will appear after the configuration) "
	"exists only to capture input events.\n"
	"\n"
	"* In order to make the controller catch input it needs focus.\n"
	"\n"
	"* Key bindings are set in the corresponding state file.\n"
	"\n";

struct Config
{
	int         Frequency;
	bool        Multithreaded;
	bool        ResourceDestructionTest;
	bool        StressTest;
	bool        DefaultState;
	bool        NetworkTest;
	bool        IsServer;
	std::string Ip;
	int         Port;

	typedef std::map<std::string, std::string> StateContainer;

	StateContainer States;
};

Config g_Config;

struct EventNetter : BFG::Emitter
{
	EventNetter(EventLoop* loop) :
	Emitter(loop)
	{
		for (size_t i=ID::A_FIRST_CONTROLLER_ACTION + 1; i<ID::A_LAST_CONTROLLER_ACTION; ++i)
		{
			loop->connect(i, this, &EventNetter::eventHandler);
		}
	}

	~EventNetter()
	{
		for (size_t i=ID::A_FIRST_CONTROLLER_ACTION + 1; i<ID::A_LAST_CONTROLLER_ACTION; ++i)
		{
			this->loop()->disconnect(i, this);
		}
	}
	
	void eventHandler(Controller_::VipEvent* e)
	{
		std::stringstream ss;
		boost::archive::text_oarchive oa(ss);
		oa << e->getData();
		CharArray512T ca512 = stringToArray<512>(ss.str());

		Network::DataPayload payload(e->getId(), 0, 0, ss.str().size(), ca512);
		emit<Network::DataPacketEvent>(ID::NE_SEND, payload);
	}
};

struct NetEventer : BFG::Emitter
{
	NetEventer(EventLoop* loop) :
	Emitter(loop)
	{
		loop->connect(BFG::ID::NE_RECEIVED, this, &NetEventer::eventHandler);
	}

	~NetEventer()
	{
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
	}

	void eventHandler(BFG::Network::DataPacketEvent* npe)
	{
		BFG::Network::DataPayload payload = npe->getData();

		std::stringstream ss;
		ss.str(payload.mAppData.data());
		boost::archive::text_iarchive ia(ss);

		Controller_::VipPayloadT vp;
		ia >> vp;

		emit<Controller_::VipEvent>(payload.mAppEventId, vp);
	}
};

template <typename TestProgramPolicy>
static void startTestProgram(TestProgramPolicy& TPP)
{
	using namespace boost::posix_time;
	using namespace Controller_;
	
	const Config& c = g_Config;

	Emitter emitter(TPP.eventLoop());

	try
	{
		boost::shared_ptr<EventNetter> en;
		boost::shared_ptr<NetEventer> ne;
		
		if (c.NetworkTest)
		{
			if (c.IsServer)
			{
				dbglog << "Emitting ID::NE_LISTEN";
				emitter.emit<BFG::Network::ControlEvent>(BFG::ID::NE_LISTEN, static_cast<u16>(c.Port));

				ne.reset(new NetEventer(emitter.loop()));
			}
			else
			{
				std::stringstream ss;
				ss << c.Port;
				CharArray128T port = stringToArray<128>(ss.str());
				CharArray128T ip = stringToArray<128>(c.Ip.c_str());
				
				dbglog << "Emitting ID::NE_CONNECT";
				emitter.emit<Network::ControlEvent>(BFG::ID::NE_CONNECT, boost::make_tuple(ip, port));
				
				en.reset(new EventNetter(emitter.loop()));
			}
			
		}
		
		
		infolog << "Creating input window";

#if defined(_WIN32)
		g_win = createInputGrabbingWindow();
		size_t win = reinterpret_cast<size_t>(g_win);
#elif defined (linux) || defined (__linux)
		createInputGrabbingWindow();
		size_t win = static_cast<size_t>(w);
#else
#error Implement this platform.
#endif

		boost::shared_ptr<OIS::InputManager> IM
		(
			Utils::CreateInputManager(win, true),
			&Utils::DestroyInputManager
		);

		Controller aController(TPP.eventLoop());
		aController.init(c.Frequency);

		ActionMapT actions;
		fillWithDefaultActions(actions);
		sendActionsToController(emitter.loop(), actions);

		// Load all states
		Path path;

		std::vector<GameHandle> state_handles;

		Config::StateContainer::const_iterator it = c.States.begin();
		for (; it != c.States.end(); ++it)
		{
			GameHandle handle = generateHandle();
			state_handles.push_back(handle);
		
			const std::string config_path = path.Expand(it->second);
			const std::string state_name = it->first;
			
			BFG::View::WindowAttributes wa;
			wa.mWidth = wa.mHeight = 50;
			wa.mHandle = win;
			StateInsertion si(config_path, state_name, handle, false, wa);

			emitter.emit<ControlEvent>(ID::CE_LOAD_STATE, si);
		}

		infolog << "All states activated.";

		ptime timestamp(microsec_clock::local_time());

		const float TEST_TIME = 10;

		size_t loopCounter  = 0;
		size_t stateCounter = 0;

		while (TPP.RunCondition())
		{
			updateInputGrabbingWindow();
		
			time_period aPeriod(timestamp, microsec_clock::local_time());
			
			++loopCounter;

			aController.nextTick();

			// When time is up, switch to next state and zero counters
			if (aPeriod.length().total_microseconds() > 1000000 * TEST_TIME)
			{
				infolog << "\n--- Loops per second (average):    "
				         << loopCounter / TEST_TIME << "\n\n\n";

				infolog << "Loop is running for "
				        << TEST_TIME << " sec...\n\n\n";

				GameHandle previousState = state_handles[stateCounter];

				++stateCounter;
				if (stateCounter == c.States.size())
					stateCounter = 0;				

				GameHandle nextState = state_handles[stateCounter];

				emitter.emit<ControlEvent>(ID::CE_DEACTIVATE_STATE, previousState);
				emitter.emit<ControlEvent>(ID::CE_ACTIVATE_STATE, nextState);

				std::string Name = boost::next(c.States.begin(), stateCounter)->first;
				infolog << "Now active: " << Name;

				loopCounter = 0;
				timestamp = microsec_clock::local_time();
			}

			TPP.AtLoopEnd();
		}
		destroyInputGrabbingWindow();
	}
	catch (std::exception& ex)
	{
		errlog << "Exception: " << ex.what();
	}
}

static bool falseIn30Seconds()
{
	using namespace boost::posix_time;
	static ptime start(microsec_clock::local_time());
	time_period period(start, microsec_clock::local_time());
	return period.length().total_seconds() < 30;
}

class EventSystemTestPolicy
{
public:
	EventSystemTestPolicy(EventLoop* Loop) :
		m_Loop(Loop)
	{}

	bool RunCondition()
	{
		if (g_Config.ResourceDestructionTest)
			return falseIn30Seconds();

		return !m_Loop->shouldExit();
	}

	EventLoop* eventLoop() const
	{
		return m_Loop;
	}

	void AtLoopEnd()
	{
		m_Loop->doLoop();
	}
	
private:
	EventLoop* m_Loop;
};

static void * EventLoopEntryPoint(void * iPointer)
{
	EventLoop * iLoop = reinterpret_cast<EventLoop*> (iPointer);
	if (iLoop)
	{
		EventSystemTestPolicy estp(iLoop);
		startTestProgram(estp);
		iLoop->setExitFlag();
		iLoop->doLoop();
	}
	return 0;
}

static void * EventLoopOutputEP(void * iPointer)
{
	EventLoop * iLoop = reinterpret_cast<EventLoop*> (iPointer);
	if (iLoop)
	{
		dbglog << "Output Thread Running";
		Controller_::EventDumper* iDump = new Controller_::EventDumper();
		
		int first = ID::A_FIRST_CONTROLLER_ACTION + 1;
		int last = ID::A_LAST_CONTROLLER_ACTION;
		
		for (int i = first; i < last; ++i)
		{
			iLoop->connect
			(
				i,
				iDump,
				static_cast
				<
					void(Controller_::EventDumper::*)(Controller_::VipEvent*)
				>
				(
					&Controller_::EventDumper::Dump
				)
			);
		}
	}
	return 0;
}

class EventLoopScheduler
{
public:
	void DoTick(LoopEvent * /*event*/)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1));
	}
};

bool isYes(char c)
{
	return (c & 223) == 'Y';
}

template <typename T>
void convertValue(const std::string& answer, T& result)
{
	std::stringstream ss(answer);
	ss >> result;
}

void convertValue(const std::string& answer, bool& result)
{
	std::stringstream ss(answer);
	ss >> std::ws;
	ss << answer;
	result = isYes(ss.str()[0]);
}

template <typename T, typename DefValueT>
void assignDefaultValue(DefValueT& default_value, T& result)
{
	result = default_value;
}

void assignDefaultValue(char default_value, bool& result)
{
	result = isYes(default_value);
}

template <typename T, typename DefValueT>
void Ask(const std::string& feature,
         T& result,
         DefValueT default_value,
         const char description[])
{
	std::cout << feature
	          << " [" << default_value << "] : ";

	std::string answer;
	std::getline(std::cin, answer);

	if (answer == "?")
	{
		std::cout << description << std::endl;
		Ask(feature, result, default_value, description);
	}
	else if (answer.size())
	{
		convertValue(answer, result);
	}
	else
	{
		assignDefaultValue(default_value, result);
	}
}

void AskStates(Config::StateContainer& Result)
{
	bool continue_to_ask = true;
	while (continue_to_ask)
	{
		std::string state;
		Ask<std::string>("Controller config to insert", state, "end",
		                 "Type a state name now (without double quotes)"
		                 " or finish your list by simply hitting <ENTER>.");

		if (state != "end")
		{
			std::string file = state;
			file.append(".xml");
			Result.insert(std::make_pair(state, file));
		}
		else
			continue_to_ask = false;
	}
}

void ConfigurateByQuestions(Config& c)
{
	std::cout << "Configuration:\n\n"
	             "* Type '?' for help.\n"
	             "\n"
	             "* Press <Enter> to use the default value in brackets.\n"
	             "\n\n";

	Ask("Controller frequency", c.Frequency, DEFAULT_FREQUENCY,
	    "Determines how often per second the Controller captures input"
	    " from its devices.");

	Ask("Do network test?", c.NetworkTest, 'n',
		"TODO: Write description.");

	if (c.NetworkTest)
	{
		Ask("Server?", c.IsServer, 'n',
			"Saying \"yes\" here will start a server, \"no\" means client.");

		if (! c.IsServer)
		{
			Ask("Ip?", c.Ip, "127.0.0.1",
				"Destination IP.");
		}

		Ask("Port?", c.Port, 1337,
			"This is the port the client will connect to, "
			"or the server will listening on.");
	}

	Ask("Use multi-threading?", c.Multithreaded, 'n',
	    "If the EventSystem will use multiple threads or not.");

	Ask("Perform resource destruction test?", c.ResourceDestructionTest, 'n',
	    "If you choose yes, the application will destroy itself after 30"
	    " seconds. This can be used to check if resources allocated by the"
	    " controller get free'd or not.");

#if 0
	Ask("Perform stress test? (not yet implemented)", c.StressTest, 'n',
		"If you choose yes, an automatic stress test will be run in order"
		" to find memory leaks and performance issues. User input gets"
	    " deactivated and random input will be thrown at the controller.");
#endif

	Ask("Use default state?", c.DefaultState, 'y',
		"If you choose yes, the state \"Console\" will be used,"
	    " assuming that it exists. If you decide otherwise (n), you'll be"
	    " asked to type a list of states you'd like to use. As of this writing"
		" there're three states available: \"GamePlayState\", \"GameRaceState\""
		" and \"MainMenuState\". The program will probably crash if you try to"
		" use non-existing states. Identifiers are case-sensitive. Confirm"
		" with <ENTER> when your list is complete.");

	if (! c.DefaultState)
		AskStates(c.States);

	if (c.States.empty())
		c.States["Console"] = "Console.xml";	
}

void startSingleThreaded()
{
	Base::CEntryPoint ep1(EventLoopOutputEP);
	Base::CEntryPoint ep2(EventLoopEntryPoint);

	EventLoop testLoop(false);

	if (g_Config.NetworkTest)
	{
		infolog << "Starting network test as " << (g_Config.IsServer?"Server":"Client");

		if (g_Config.IsServer)
		{
			testLoop.addEntryPoint(BFG::Network::Interface::getEntryPoint(BFG_SERVER));
		}
		else
		{
			testLoop.addEntryPoint(BFG::Network::Interface::getEntryPoint(BFG_CLIENT));
		}
	}
	
	testLoop.addEntryPoint(&ep1);
	testLoop.addEntryPoint(&ep2);

	testLoop.run();
}

void startMultiThreaded()
{
	EventLoop testLoop
	(
		false,
		new EventSystem::BoostThread<>("Test Function"),
		new EventSystem::InterThreadCommunication()
	);

	if (g_Config.NetworkTest)
	{
		infolog << "Starting network test as " << (g_Config.IsServer?"Server":"Client");

		if (g_Config.IsServer)
		{
			testLoop.addEntryPoint(BFG::Network::Interface::getEntryPoint(BFG_SERVER));
		}
		else
		{
			testLoop.addEntryPoint(BFG::Network::Interface::getEntryPoint(BFG_CLIENT));
		}
	}
	
	testLoop.addEntryPoint(new Base::CEntryPoint(EventLoopEntryPoint));
	testLoop.run();

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	EventLoopScheduler iSleeper;

	EventLoop testLoop2
	(
		true,
		new EventSystem::BoostThread<>("EventDumper"),
		new EventSystem::InterThreadCommunication()
	);
	testLoop2.addEntryPoint(new Base::CEntryPoint(EventLoopOutputEP));
	testLoop2.registerLoopEventListener(&iSleeper, &EventLoopScheduler::DoTick);
	testLoop2.run();

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	BFG::Base::pause();
	testLoop.setExitFlag();
	testLoop2.setExitFlag();
	boost::this_thread::sleep(boost::posix_time::milliseconds(100));
}

int main(int, char**) try
{
	Path p;
	const std::string logPath = p.Get(ID::P_LOGS) + "ControllerTest.log";
	Base::Logger::Init(Base::Logger::SL_DEBUG, logPath);

	infolog << PROGRAM_DESCRIPTION;
	
	ConfigurateByQuestions(g_Config);

	if (g_Config.Multithreaded)
	{
		startMultiThreaded();
	}
	else
	{
		startSingleThreaded();
	}

	return 0;
}
catch (std::exception& ex)
{
	std::cerr << "Exception: " << ex.what() << std::endl;
	Base::pause();
}
catch (...)
{
	std::cerr << "Catched unknown exception (aborting)." << std::endl;
	Base::pause();
}
