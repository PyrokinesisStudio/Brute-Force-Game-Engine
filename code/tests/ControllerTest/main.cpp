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

#include <Base/LibraryMainBase.h>
#include <Base/Logger.h>
#include <Base/Cpp.h>
#include <Base/Pause.h>
#include <Base/ResolveDns.h>

#include <Core/Path.h>
#include <Core/GameHandle.h>
#include <Core/Enums.hh>

#include <Controller/Controller.h>
#include <Event/Event.h>

#ifdef BFG_USE_NETWORK
#include <Network/Network.h>
#endif

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

#ifdef BFG_USE_NETWORK
struct EventNetter
{
	EventNetter(Event::Lane& eventLane)
	{
		for (size_t i=ID::A_FIRST_CONTROLLER_ACTION + 1; i<ID::A_LAST_CONTROLLER_ACTION; ++i)
		{
			eventLane.connect(i, this, &EventNetter::eventHandler);
		}
	}

	~EventNetter()
	{}
	
	void eventHandler(Controller_::VipEvent* e)
	{
		std::stringstream ss;
		boost::archive::text_oarchive oa(ss);
		oa << e->data();
		CharArray512T ca512 = stringToArray<512>(ss.str());

		Network::DataPayload payload(e->id(), 0, 0, ss.str().size(), ca512);
		emit<Network::DataPacketEvent>(ID::NE_SEND, payload);
	}
};

struct NetEventer
{
	NetEventer(Event::Lane& eventLane) :
	Emitter(eventLane)
	{
		eventLane.connect(BFG::ID::NE_RECEIVED, this, &NetEventer::eventHandler);
	}

	~NetEventer()
	{}

	void eventHandler(BFG::Network::DataPacketEvent* npe)
	{
		BFG::Network::DataPayload payload = npe->data();

		std::stringstream ss;
		ss.str(payload.mAppData.data());
		boost::archive::text_iarchive ia(ss);

		Controller_::VipPayloadT vp;
		ia >> vp;

		emit<Controller_::VipEvent>(payload.mAppEventId, vp);
	}
};
#endif

static bool falseIn30Seconds()
{
	using namespace boost::posix_time;
	static ptime start(microsec_clock::local_time());
	time_period period(start, microsec_clock::local_time());
	return period.length().total_seconds() < 30;
}

struct Main : Base::LibraryMainBase<Event::Lane>
{
	Main() :
	mEventLane(NULL),
	mStateCounter(0),
	mLoopCounter(0)
	{}
	
	void main(Event::Lane* eventLane)
	{
		std::cout << "Main::main" << std::endl;
		mEventLane = eventLane;
		eventLane->connectLoop(this, &Main::loopEventHandler);
		eventLane->connect(ID::A_KEY_PRESSED, this, &Main::onButtonPress);
		startTestProgram();
		std::cout << "End Main" << std::endl;
	}
	
	void stop()
	{
		destroyInputGrabbingWindow();
	}
	
	void onButtonPress(s32 value)
	{
		std::cout << "Main::onButtonPress(" << value << ")" << std::endl;
	}
	
	void startTestProgram()
	{
		std::cout << "startTestProgram" << std::endl;
		using namespace boost::posix_time;
		using namespace Controller_;
		
		try
		{
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

			ActionMapT actions;
			fillWithDefaultActions(actions);
			sendActionsToController(*mEventLane, actions);

			// Load all states
			Path path;

			Config::StateContainer::const_iterator it = g_Config.States.begin();
			for (; it != g_Config.States.end(); ++it)
			{
				GameHandle handle = generateHandle();
				mStateHandles.push_back(handle);
			
				const std::string config_path = path.Expand(it->second);
				const std::string state_name = it->first;
				
				BFG::View::WindowAttributes wa;
				wa.mWidth = wa.mHeight = 50;
				wa.mHandle = win;
				StateInsertion si(config_path, state_name, handle, false, wa);

				mEventLane->emit(ID::CE_LOAD_STATE, si);
			}

			infolog << "All states activated.";

		}
		catch (std::exception& ex)
		{
			errlog << "Exception: " << ex.what();
		}
	}

	void loopEventHandler(Event::TickData)
	{
		using namespace boost::posix_time;
		using namespace Controller_;
		
		static ptime timestamp(microsec_clock::local_time());

		static const float TEST_TIME = 10;

		updateInputGrabbingWindow();
	
		time_period aPeriod(timestamp, microsec_clock::local_time());
		
		++mLoopCounter;

		// When time is up, switch to next state and zero counters
		if (aPeriod.length().total_microseconds() > 1000000 * TEST_TIME)
		{
			infolog << "\n--- Loops per second (average):    "
			         << mLoopCounter / TEST_TIME << "\n\n\n";

			infolog << "Loop is running for "
			        << TEST_TIME << " seconds ...\n\n\n";

			GameHandle previousState = mStateHandles[mStateCounter];

			++mStateCounter;
			if (mStateCounter == mStateHandles.size())
				mStateCounter = 0;

			GameHandle nextState = mStateHandles[mStateCounter];

			mEventLane->emit(ID::CE_DEACTIVATE_STATE, previousState);
			mEventLane->emit(ID::CE_ACTIVATE_STATE, nextState);

			std::string Name = boost::next(g_Config.States.begin(), mStateCounter)->first;
			infolog << "Now active: " << Name;

			mLoopCounter = 0;
			timestamp = microsec_clock::local_time();
		}
	}
	
	std::vector<GameHandle> mStateHandles;
	Event::Lane* mEventLane;
	size_t mStateCounter;
	size_t mLoopCounter;
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

	Ask("Controller frequency", g_Config.Frequency, DEFAULT_FREQUENCY,
	    "Determines how often per second the Controller captures input"
	    " from its devices.");

#ifdef BFG_USE_NETWORK
	Ask("Do network test?", g_Config.NetworkTest, 'n',
		"TODO: Write description.");

	if (g_Config.NetworkTest)
	{
		Ask("Server?", g_Config.IsServer, 'n',
			"Saying \"yes\" here will start a server, \"no\" means client.");

		if (! g_Config.IsServer)
		{
			Ask("Ip?", g_Config.Ip, "127.0.0.1",
				"Destination IP.");
		}

		Ask("Port?", g_Config.Port, 1337,
			"This is the port the client will connect to, "
			"or the server will listening on.");
	}
#endif

	Ask("Use multi-threading?", g_Config.Multithreaded, 'n',
	    "If the EventSystem will use multiple threads or not.");

	Ask("Perform resource destruction test?", g_Config.ResourceDestructionTest, 'n',
	    "If you choose yes, the application will destroy itself after 30"
	    " seconds. This can be used to check if resources allocated by the"
	    " controller get free'd or not.");

#if 0
	Ask("Perform stress test? (not yet implemented)", g_Config.StressTest, 'n',
		"If you choose yes, an automatic stress test will be run in order"
		" to find memory leaks and performance issues. User input gets"
	    " deactivated and random input will be thrown at the controller.");
#endif

	Ask("Use default state?", g_Config.DefaultState, 'y',
		"If you choose yes, the state \"Console\" will be used,"
	    " assuming that it exists. If you decide otherwise (n), you'll be"
	    " asked to type a list of states you'd like to use. As of this writing"
		" there're three states available: \"GamePlayState\", \"GameRaceState\""
		" and \"MainMenuState\". The program will probably crash if you try to"
		" use non-existing states. Identifiers are case-sensitive. Confirm"
		" with <ENTER> when your list is complete.");

	if (! g_Config.DefaultState)
		AskStates(g_Config.States);

	if (g_Config.States.empty())
		g_Config.States["Console"] = "Console.xml";	
}

void startSingleThreaded()
{
	Event::Synchronizer sync;
	Event::Lane controllerLane(sync, 100);
	
	
#ifdef BFG_USE_NETWORK
	boost::scoped_ptr<BFG::Network::Main> networkMain;

	if (g_Config.NetworkTest)
	{
		infolog << "Starting network test as " << (g_Config.IsServer?"Server":"Client");

		if (g_Config.IsServer)
		{
			networkMain.reset(new BFG::Network::Main(&testLoop, BFG_SERVER));
		}
		else
		{
			networkMain.reset(new BFG::Network::Main(&testLoop, BFG_CLIENT));
		}
		
		testLoop.addEntryPoint(networkMain->entryPoint());
	}
#endif

	sync.finish();
}

void startMultiThreaded()
{
	u32 frequency = 100;
	Event::Synchronizer sync;
	Event::Lane controllerLane(sync, frequency);
	
	controllerLane.addEntry<Controller_::Main>(frequency);
	controllerLane.addEntry<Main>();
	sync.start();
	
	boost::this_thread::sleep(boost::posix_time::seconds(35));
	
#ifdef BFG_USE_NETWORK
	boost::scoped_ptr<BFG::Network::Main> networkMain;

	if (g_Config.NetworkTest)
	{
		infolog << "Starting network test as " << (g_Config.IsServer?"Server":"Client");

		if (g_Config.IsServer)
		{
			networkMain.reset(new BFG::Network::Main(&testLoop, BFG_SERVER));
		}
		else
		{
			networkMain.reset(new BFG::Network::Main(&testLoop, BFG_CLIENT));
		}
	}
	
	testLoop.addEntryPoint(networkMain->entryPoint());
#endif

	sync.finish();
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
