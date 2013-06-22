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

#include <EventSystem/Event.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>
#include <Core/GameHandle.h>
#include <Core/ClockUtils.h>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>

const EventIdT testEventId = 5;
const BFG::GameHandle testDestinationId = 1234;
const BFG::GameHandle testSenderId = 5678;

bool o1 = false;
bool o2 = false;

// ---------------------------------------------------------------------------
struct HelloWorld
{
	void operator()() const
	{
		std::cout << "operator(), World!" << std::endl;
	}
	
	void test() const
	{
		std::cout << "test, Test!" << std::endl;
	}
	
	void data(const std::string& s) const
	{
		std::cout << "data, " << s << std::endl;
	}
	
	void data2(const std::vector<std::string>& v) const
	{
		BOOST_FOREACH(const std::string& s, v)
		{
			std::cout << "data2, " << s << std::endl;
		}
	}
	void data3(const std::vector<std::string>& v) const
	{
		BOOST_FOREACH(const std::string& s, v)
		{
			std::cout << "data3, " << s << std::endl;
		}
	}
	
	void other1(const std::string& s) const
	{
		std::cout << "other1, " << s << std::endl;
		o1 = true;
	}
	void other2(const std::string& s) const
	{
		std::cout << "other2, " << s << std::endl;
		o2 = true;
	}
};

struct Callable
{
	virtual ~Callable()
	{}
	
	virtual void call() = 0;
};

template <typename PayloadT>
struct Binding : public Callable
{
	typedef boost::signals2::signal<void (const PayloadT&)> SignalT;
	
	Binding() :
	mSignal(new SignalT),
	mTypeInfo(&typeid(PayloadT))
	{}
	
	template <typename FnT>
	void connect(FnT fn)
	{
		mSignal->connect(fn);
	}

	void emit(const PayloadT& payload)
	{
		if (typeid(PayloadT) != *mTypeInfo)
			throw std::runtime_error("Incompatible types");
		
		mPayloads.push_back(payload);
	}
	
	virtual void call()
	{
		BOOST_FOREACH(const PayloadT& payload, mPayloads)
		{
			signal()(payload);
		}
		//mPayloads.clear();
	}
	
	const SignalT& signal() const
	{
		return *mSignal;
	}

	std::vector<PayloadT> mPayloads;
	boost::shared_ptr<SignalT> mSignal;
	const std::type_info* mTypeInfo;
};

// Säule
struct EventBinder
{
	template <typename PayloadT, typename FnT>
	void connect(int id, FnT fn)
	{
		if (mSignals.count(id) == 0)
		{
			Callable* c = new Binding<PayloadT>();
			mSignals[id] = c;
		}
		
		Callable* c = boost::any_cast<Callable*>(mSignals[id]);
		Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
		b->connect(fn);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(int id, const PayloadT& payload)
	{
		if (mSignals.find(id) != mSignals.end())
		{
			Callable* c = boost::any_cast<Callable*>(mSignals[id]);
			Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
			b->emit(payload);
		}
		//! \todo Else: event id not found in this EventBinder
	}

	// Callt alle Handler für ein Event
	void call(int id)
	{
		//! \todo Das hier in tick() rein?
		Callable* c = boost::any_cast<Callable*>(mSignals[id]);
		c->call();
	}
	
	// Verarbeitet alle events, die mit emit() gequeued wurden.
	void tick()
	{
		BOOST_FOREACH(SignalMapT::value_type& pair, mSignals)
		{
			call(pair.first);
		}
	}
	
	// Welche Events können von dieser Säule abgearbeitet werden?
	// Welche Callback-Funktionen existieren für diese Säule?
	typedef std::map<int, boost::any> SignalMapT;

	SignalMapT mSignals;
};

struct Pillar;

struct Synchronizer
{
	Synchronizer(int nThreads) :
	mNumberThreads(nThreads)
	{}
	
	void add(Pillar* pillar);

private:
	void loop(Pillar* pillar);

	int mNumberThreads;
	std::vector<Pillar*> mPillars;
	std::vector<boost::shared_ptr<boost::thread> > mThreads;
};

struct Pillar
{
	Pillar(Synchronizer& synchronizer, int ticksPerSecond) :
	mSynchronizer(synchronizer),
	mTicksPerSecond(ticksPerSecond)
	{
		synchronizer.add(this);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(int id, const PayloadT& payload)
	{
		mEventBinder.emit<PayloadT>(id, payload);
	}
/*
	template <typename FnT>
	void connect(int id, FnT fn)
	{
		typedef typename boost::function_traits<FnT>::arg1_type PayloadT; 
		mEventBinder.connect<PayloadT>(id, fn);
	}
*/
	template <typename PayloadT, typename FnT>
	void connect(int id, FnT fn)
	{
		//typedef typename boost::function_traits<FnT>::arg1_type PayloadT; 
		mEventBinder.connect<PayloadT>(id, fn);
	}
	// audio.connect(1, &HelloWorld::other1, hello);
//	template <typename FnT, typename ObjectT>
//	void connect(int id, FnT fn, ObjectT& obj)
//	template <typename PayloadT, typename EventListener>
//	void connect(int id, void(EventListener::*functionPointer)(PayloadT event) const, EventListener& listener)
//	{
		
		//boost::function<void()> f;
//		typedef typename boost::remove_pointer<FnT>::type FnT2;
//		typedef typename boost::function_traits<FnT3>::template arg<0>::type PayloadT;
		//mEventBinder.connect<PayloadT>(id, boost::bind(fn, boost::ref(obj), _1));
//	}

	void tick()
	{
		mEventBinder.tick();
	}
	
	void waitRemainingTime(int consumedTime)
	{
		BFG::s32 plannedTime = 1000 / mTicksPerSecond;
		BFG::s32 remainingTime = plannedTime - consumedTime;
		if (remainingTime > 0)
		{
			std::cout << "Thread " << boost::this_thread::get_id() << " sleeping " << remainingTime << std::endl;
			boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
		}
	}
	
private:
	int mTicksPerSecond;
	Synchronizer& mSynchronizer;
	EventBinder mEventBinder;
};

void Synchronizer::add(Pillar* pillar)
{
	mPillars.push_back(pillar);
	mThreads.push_back(boost::make_shared<boost::thread>(boost::bind(&Synchronizer::loop, this, pillar)));
	boost::thread t;
}

void Synchronizer::loop(Pillar* pillar)
{
	BFG::Clock::StopWatch sw(BFG::Clock::milliSecond);

	//! \todo stop condition
	for (;;)
	{
		sw.start();
		pillar->tick();
		BFG::s32 passedTime = sw.stop();

		pillar->waitRemainingTime(passedTime);
	}
}

BOOST_AUTO_TEST_SUITE(TestSuite)

BOOST_AUTO_TEST_CASE (Test)
{
	EventBinder sh;
	EventBinder sh2;

	// Connect a HelloWorld slot
	HelloWorld hello;
/*	
	sh.connect<std::string>(1, boost::bind(&HelloWorld::data, boost::ref(hello), _1));
	sh.emit(1, std::string("s1"));
	sh.emit(1, std::string("s1"));
	
	sh.connect<std::vector<std::string> >(2, boost::bind(&HelloWorld::data2, boost::ref(hello), _1));
	sh.connect<std::vector<std::string> >(2, boost::bind(&HelloWorld::data3, boost::ref(hello), _1));
	
	sh2.connect<std::string>(3, boost::bind(&HelloWorld::data, boost::ref(hello), _1));

	std::vector<std::string> v;
	v.push_back("v a");
	v.push_back("v b");
	v.push_back("v c");
	sh.emit(2, v);
	
	std::vector<std::string> v2;
	v2.push_back("v2 a");
	v2.push_back("v2 b");
	sh.emit(2, v2);

	sh2.emit(3, std::string("s3"));
	
	//BOOST_CHECK_THROW(sh.emit(1, v2), std::runtime_error);
*/	

	const int nThreads = 2;
	const int ticksPerSecond100 = 100;
	const int ticksPerSecond5 = 5;
	Synchronizer sync(nThreads);
	Pillar audio(sync, ticksPerSecond5);
	Pillar physics(sync, ticksPerSecond100);
	
	audio.connect<std::string>(1, boost::bind(&HelloWorld::other1, boost::ref(hello), _1));
	physics.connect<std::string>(1, boost::bind(&HelloWorld::other2, boost::ref(hello), _1));
	
	//audio.connect<std::string>(1, &HelloWorld::other1, hello);
	for (int i=0; i<100; ++i)
	{
		physics.emit(1, std::string("Boom"));
		audio.emit(1, std::string("Boom2"));
		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	}

//	sh.connect<std::string>(1, boost::bind(&HelloWorld::other1, boost::ref(hello), _1));
//	sh2.connect<std::string>(1, boost::bind(&HelloWorld::other2, boost::ref(hello), _1));

//	sh.emit(1, std::string("1"));
	
	//boost::thread t1(boost::bind(&loop, boost::ref(sh)));
	//loop(sh2);
	
	//t1.join();
	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	
	BOOST_CHECK_EQUAL(o1, true);
	BOOST_CHECK_EQUAL(o2, true);
	
/*
	sync client;
	sync server;

	object Audio(client);
	object GameC(client); // current thread
	object GameS(server);
	object NetworkC(client);
	object NetworkS(server);
	object Physics(server);
	object View(client);
	
	Physics.emit(POSITION, "123");
	View.connect(POSITION, &onPosition);
*/
}

BOOST_AUTO_TEST_SUITE_END()
