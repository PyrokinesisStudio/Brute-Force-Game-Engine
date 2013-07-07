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

#if defined (_WIN32)
#define BOOST_PARAMETER_MAX_ARITY 7
#endif

#include <EventSystem/Event.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>
#include <Core/GameHandle.h>
#include <Core/ClockUtils.h>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>

const EventIdT testEventId = 5;
const BFG::GameHandle testDestinationId = 1234;
const BFG::GameHandle testSenderId = 5678;

bool o1 = false;
bool o2 = false;

template <typename _EventIdT, typename _DestionationIdT>
struct BasicEventAddress
{
	typedef _EventIdT EventIdT;
	typedef _DestionationIdT DestionationIdT;

	EventIdT mEventId;
	DestionationIdT mDestinationId;
	boost::any mBinding;
};

typedef BasicEventAddress<int,int> EventAddress;


typedef boost::multi_index_container
<
	EventAddress, 
	boost::multi_index::indexed_by
	<
		boost::multi_index::ordered_unique
		<
			boost::multi_index::composite_key
			< 
				EventAddress,
				boost::multi_index::member
				<
					EventAddress,
					EventAddress::EventIdT,
					&EventAddress::mEventId
				>,
				boost::multi_index::member
				<
					EventAddress,
					EventAddress::DestionationIdT,
					&EventAddress::mDestinationId
				>
			>
		>
	>
> EventAddressMapT;

struct LoopData
{
	LoopData(BFG::s32 tslt) :
	mTimeSinceLastTick(tslt)
	{}

	BFG::s32 mTimeSinceLastTick;
};

struct Pillar;
// ---------------------------------------------------------------------------
struct HelloWorld
{
	HelloWorld(Pillar* pillar, int id) :
	c1(0),
	c2(0),
	c3(0),
	c4(0),
	mPillar(pillar),
	mId(id)
	{}
	
	~HelloWorld()
	{
		std::cout << std::dec << "Received " << c1+c2+c3+c4 << " events.\n";
	}
	
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
	
	void other1(const std::string& s);
	
	void other2(const std::string& s)
	{
		std::cout << "physics (" << c2 << "), " << s << std::endl;
		++c2;
	}
	void other3(const std::string& s)
	{
		std::cout << "view (" << c3 << "), " << s << std::endl;
		++c3;
	}
	void other4(const std::string& s)
	{
		std::cout << "game (" << c4 << "), " << s << std::endl;
		++c4;
	}
	
	void update(const LoopData ld)
	{
		std::cout << "++++++++++update+++++++++++(" << ld.mTimeSinceLastTick << ")" << std::endl;
	}

	int c1,c2,c3,c4,mId;
	Pillar* mPillar;
};

struct Callable
{
	virtual ~Callable()
	{}
	
	virtual void call() = 0;
};

//! This is the binding of a collection of functions (boost::signals) with a collection of payloads
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
		boost::mutex::scoped_lock sl(mFlipLocker);
		
		if (typeid(PayloadT) != *mTypeInfo)
			throw std::runtime_error("Incompatible types");
		
		mBackPayloads.push_back(payload);
	}
	
	virtual void call()
	{
		flipPayloads();
		BOOST_FOREACH(const PayloadT& payload, mFrontPayloads)
		{
			signal()(payload);
		}
		mFrontPayloads.clear();
	}

	void flipPayloads()
	{
		boost::mutex::scoped_lock sl(mFlipLocker);
		std::swap(mFrontPayloads, mBackPayloads);
	}

private:
	const SignalT& signal() const
	{
		return *mSignal;
	}

	std::vector<PayloadT> mFrontPayloads;
	std::vector<PayloadT> mBackPayloads;

	boost::shared_ptr<SignalT> mSignal;
	const std::type_info* mTypeInfo;
	
	boost::mutex mFlipLocker;
};

//! The EventBinder holds the map of EventIds with its corresponding Bindings 
struct EventBinder
{
	template <typename PayloadT, typename FnT>
	void connect(int id, FnT fn, const int destination)
	{
		Callable* c = NULL;
	//	Binding<PayloadT>* c = NULL;
		EventAddressMapT::iterator it = mSignals.find(boost::make_tuple(id, destination));
		if (it == mSignals.end())
		{
			c = new Binding<PayloadT>(); 
			EventAddress ea = {id, destination, c};
			mSignals.insert(ea);
		}
		else
		{
			c = boost::any_cast<Callable*>(it->mBinding);
		}

// 		Callable* c = boost::any_cast<Callable*>(mSignals[id]);
//
		Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
		b->connect(fn);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(int id, const PayloadT& payload, const int destination)
	{
		EventAddressMapT::iterator it = mSignals.find(boost::make_tuple(id, destination));
		if (it != mSignals.end())
		{
			Callable* c = boost::any_cast<Callable*>(it->mBinding);
			Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
			b->emit(payload);
		}
		//! \todo Else: event id not found in this EventBinder
	}

	// Callt alle Handler für ein Event
// 	void call(int id)
// 	{
// 		//! \todo Das hier in tick() rein?
// 		Callable* c = boost::any_cast<Callable*>(mSignals[id]);
// 		c->call();
// 	}
	
	// Verarbeitet alle events, die mit emit() gequeued wurden.
	void tick()
	{
		BOOST_FOREACH(const EventAddressMapT::value_type& addressMapValue, mSignals)
		{
			Callable* c = boost::any_cast<Callable*>(addressMapValue.mBinding);
			c->call();
		}
	}
	
	// Welche Events können von dieser Säule abgearbeitet werden?
	// Welche Callback-Funktionen existieren für diese Säule?
//	typedef std::map<int, boost::any> SignalMapT;

	EventAddressMapT mSignals;
};

struct Pillar;

struct Synchronizer
{
	Synchronizer(int nThreads) :
	mNumberThreads(nThreads),
	mFinishing(false)
	{}

	~Synchronizer();
	
	void add(Pillar* pillar);
	
	void finish();
	
	template <typename PayloadT>
	void distributeToOthers(int id, const PayloadT& payload, Pillar* pillar, const int destination = 0);

private:
	void loop(Pillar* pillar);

	int mNumberThreads; //! \todo unused
	std::vector<Pillar*> mPillars;
	std::vector<boost::shared_ptr<boost::thread> > mThreads;
	
	bool mFinishing;
	
	std::vector<boost::shared_ptr<boost::barrier> > mBarrier;
};

struct Pillar
{
	Pillar(Synchronizer& synchronizer, BFG::s32 ticksPerSecond) :
	mSynchronizer(synchronizer),
	mTicksPerSecond(ticksPerSecond),
	sw(BFG::Clock::milliSecond)
	{
		sw.start();
		mSynchronizer.add(this);
	}
	
	// Speichert ein Payload, das später ausgeliefert wird an einen Handler.
	template <typename PayloadT>
	void emit(int id, const PayloadT& payload, const int destination = 0)
	{
		//boost::mutex::scoped_lock sl(mEmitLocker);
		mEventBinder.emit<PayloadT>(id, payload, destination);
		mSynchronizer.distributeToOthers(id, payload, this, destination);
	}
	
	template <typename PayloadT>
	void emitFromOther(int id, const PayloadT& payload, const int destination)
	{
		//boost::mutex::scoped_lock sl(mEmitLocker);
		mEventBinder.emit<PayloadT>(id, payload, destination);
	}

	template <typename FnT>
	void connectLoop(FnT loopFn)
	{
		mLoopBinding.connect(loopFn);
	}

	template <typename PayloadT, typename FnT>
	void connect(int id, FnT fn, int destination = 0)
	{
		//typedef typename boost::function_traits<FnT>::arg1_type PayloadT; 
		mEventBinder.connect<PayloadT>(id, fn, destination);
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
		mLoopBinding.emit(LoopData(sw.restart()));
		mLoopBinding.call();

		mEventBinder.tick();

		waitRemainingTime(sw.stop());
	}

	void waitRemainingTime(BFG::s32 consumedTime)
	{
		BFG::s32 plannedTime = 1000 / mTicksPerSecond;
		BFG::s32 remainingTime = plannedTime - consumedTime;
		if (remainingTime > 0)
		{
			//std::cout << "Thread " << boost::this_thread::get_id() << " sleeping " << remainingTime << std::endl;
			boost::this_thread::sleep(boost::posix_time::milliseconds(remainingTime));
		}
	}
	
private:
	BFG::Clock::StopWatch sw;
	BFG::s32 mTicksPerSecond;
	Synchronizer& mSynchronizer;
	EventBinder mEventBinder;
	boost::mutex mEmitLocker;

	Binding<LoopData> mLoopBinding;
};

Synchronizer::~Synchronizer()
{
	finish();
}

void Synchronizer::add(Pillar* pillar)
{
	mPillars.push_back(pillar);
	mThreads.push_back(boost::make_shared<boost::thread>(boost::bind(&Synchronizer::loop, this, pillar)));
	boost::thread t;
}

template <typename PayloadT>
void Synchronizer::distributeToOthers(int id, const PayloadT& payload, Pillar* pillar, const int destination)
{
	BOOST_FOREACH(Pillar* other, mPillars)
	{
		if (other != pillar)
		{
			std::cout << ".";
			other->emitFromOther(id, payload, destination);
		}
	}
}


void Synchronizer::finish()
{
	for (size_t i=0; i<10; ++i)
		mBarrier.push_back(boost::make_shared<boost::barrier>(mThreads.size()));

	mFinishing = true;
	
	BOOST_FOREACH(boost::shared_ptr<boost::thread> t, mThreads)
	{
		std::cout << "Joining thread " << t->get_id() << "\n";
		t->join();
	}
}

void Synchronizer::loop(Pillar* pillar)
{
	while (!mFinishing)
	{
		pillar->tick();
	}
	
	for (std::size_t i=0; i<mBarrier.size(); ++i)
	{
		std::cout << "Joining Barrier #" << i << " - " << boost::this_thread::get_id() << std::endl;
		mBarrier[i]->wait();
		std::cout << "TICK #" << i << " - " << boost::this_thread::get_id() << std::endl;
		pillar->tick();
	}
	// -- Thread exits here --
}

void HelloWorld::other1(const std::string& s)
{
	std::cout << "audio[" << mId << "]" << "(" << c1 << "), " << s << std::endl;
	++c1;
	boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	std::cout << "audio slept 300ms" << std::endl;
	mPillar->emit(2, std::string("To View from Audio"));
}

BOOST_AUTO_TEST_SUITE(TestSuite)

BOOST_AUTO_TEST_CASE (Test)
{
	const int nThreads = 2;
	const int ticksPerSecond100 = 100;
	const int ticksPerSecond5 = 5;
	Synchronizer sync(nThreads);
	Pillar audio(sync, ticksPerSecond5);
	Pillar physics(sync, ticksPerSecond100);
	Pillar view(sync, ticksPerSecond100);
	Pillar game(sync, ticksPerSecond100);

	// Connect a HelloWorld slot
	HelloWorld hello(&audio, 15);
	HelloWorld hello2(&audio, 16);
	
	audio.connectLoop(boost::bind(&HelloWorld::update, boost::ref(hello), _1));
	audio.connect<std::string>(1, boost::bind(&HelloWorld::other1, boost::ref(hello), _1), 15);
	audio.connect<std::string>(1, boost::bind(&HelloWorld::other1, boost::ref(hello2), _1));
	physics.connect<std::string>(1, boost::bind(&HelloWorld::other2, boost::ref(hello), _1));
	view.connect<std::string>(2, boost::bind(&HelloWorld::other3, boost::ref(hello), _1));
	game.connect<std::string>(2, boost::bind(&HelloWorld::other4, boost::ref(hello), _1));
	
	//audio.connect<std::string>(1, &HelloWorld::other1, hello);
	for (int i=0; i<2; ++i)
	{
		physics.emit(1, std::string("Physics Boom"));
		audio.emit(2, std::string("Audio Boom"));
		view.emit(2, std::string("View Boom"));
		game.emit(1, std::string("Game Boom"), 15);
		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	}
	physics.emit(1, std::string("Ende"));
	sync.finish();

}

BOOST_AUTO_TEST_SUITE_END()
