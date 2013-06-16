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
#include <ControllerTest/InputWindowX11.h>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/signals2.hpp>

const EventIdT testEventId = 5;
const BFG::GameHandle testDestinationId = 1234;
const BFG::GameHandle testSenderId = 5678;

// ---------------------------------------------------------------------------
struct HelloWorld
{
	void operator()() const
	{
		std::cout << "Hello, World!" << std::endl;
	}
	
	void test() const
	{
		std::cout << "Hello, Test!" << std::endl;
	}
	
	void data(const std::string& s) const
	{
		std::cout << "Hello, " << s << std::endl;
	}
	
	void data2(const std::vector<std::string>& v) const
	{
		BOOST_FOREACH(const std::string& s, v)
		{
			std::cout << "Hello, " << s << std::endl;
		}
	}
	void data3(const std::vector<std::string>& v) const
	{
		BOOST_FOREACH(const std::string& s, v)
		{
			std::cout << "Ok, " << s << std::endl;
		}
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
		mPayloads.clear();
	}
	
	const SignalT& signal() const
	{
		return *mSignal;
	}

	std::vector<PayloadT> mPayloads;
	boost::shared_ptr<SignalT> mSignal;
	const std::type_info* mTypeInfo;
};

struct SignalHolder
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
	
	template <typename PayloadT>
	void emit(int id, const PayloadT& payload)
	{
		Callable* c = boost::any_cast<Callable*>(mSignals[id]);
		Binding<PayloadT>* b = static_cast<Binding<PayloadT>*>(c);
		b->emit(payload);
	}

	void call(int id)
	{
		Callable* c = boost::any_cast<Callable*>(mSignals[id]);
		c->call();
	}
	
	void tick()
	{
		BOOST_FOREACH(SignalMapT::value_type& pair, mSignals)
		{
			call(pair.first);
		}
	}
	
	typedef std::map<int, boost::any> SignalMapT;
	SignalMapT mSignals;
};

BOOST_AUTO_TEST_SUITE(TestSuite)

BOOST_AUTO_TEST_CASE (Test)
{
	SignalHolder sh;
	
	// Signal with no arguments and a void return value
	boost::signals2::signal<void ()> sig;
	boost::signals2::signal<void (const std::string&)> ssig;

	// Connect a HelloWorld slot
	HelloWorld hello;
	sig.connect(hello);
	sig.connect(boost::bind(&HelloWorld::test, boost::ref(hello)));

	sh.connect<std::string>(1, boost::bind(&HelloWorld::data, boost::ref(hello), _1));
	sh.emit(1, std::string("Lala"));
	sh.emit(1, std::string("Lulu"));
	
	sh.connect<std::vector<std::string> >(2, boost::bind(&HelloWorld::data2, boost::ref(hello), _1));
	sh.connect<std::vector<std::string> >(2, boost::bind(&HelloWorld::data3, boost::ref(hello), _1));
	// Call all of the slots
	sig();
	ssig("Andreas");

	std::vector<std::string> v;
	v.push_back("Trululu");
	v.push_back("Trilili");
	v.push_back("Tralala");
	sh.emit(2, v);
	std::vector<std::string> v2;
	v2.push_back("Miau");
	v2.push_back("Wuff");
	sh.emit(2, v2);
	
	std::cout << "TICK" << std::endl;
	sh.tick();

	BOOST_CHECK_THROW(sh.emit(1, v2), std::runtime_error);

	std::cout << "TICK" << std::endl;
	sh.tick();
}

BOOST_AUTO_TEST_SUITE_END()
