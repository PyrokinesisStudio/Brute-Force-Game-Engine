#include <iostream>
#include <vector>
#include <boost/test/unit_test.hpp>

struct Cpp11Notification
{
	Cpp11Notification()
	{
		std::cout << "C++11 features enabled. Version:" << __cplusplus << std::endl;
	}
};

//! Testing move semantics: An object which can only be moved, but not copied.
struct Object : boost::noncopyable
{
	Object(const std::string& msg) : msg(msg) {}
	Object(Object&& o) : msg(std::move(o.msg)) {}
	Object& operator = (Object&& o){ msg = std::move(o.msg); return *this; }
	std::string msg;
};

//! Testing move semantics: Creates an 'Object' and returns it by moving it back.
Object createObject()
{
	return std::move(Object("Testing move semantics"));
}

BOOST_GLOBAL_FIXTURE (Cpp11Notification);

BOOST_AUTO_TEST_SUITE(Cpp11TestSuite)

BOOST_AUTO_TEST_CASE (testAuto)
{
	std::vector<int> v;
	v.push_back(5);
	auto it = v.begin();
	it = it;
}

BOOST_AUTO_TEST_CASE (testLambda)
{
	std::vector<int> v;
	v.push_back(5);
	std::for_each(v.begin(), v.end(), [&](int& i) { return i=i*i; });
}

BOOST_AUTO_TEST_CASE (testMoveSemantics)
{
	std::vector<Object> v;
	v.emplace_back(createObject());
	BOOST_CHECK_EQUAL(v.size(), 1);
	BOOST_CHECK(v[0].msg == "Testing move semantics");
}

BOOST_AUTO_TEST_CASE (testNullPtr)
{
	int* p = nullptr;
	std::cout << "nullptr: " << p << std::endl;
}

BOOST_AUTO_TEST_CASE (testStaticAssert)
{
	static_assert(true != false, "Something seriously wrong here");
}

BOOST_AUTO_TEST_SUITE_END()

