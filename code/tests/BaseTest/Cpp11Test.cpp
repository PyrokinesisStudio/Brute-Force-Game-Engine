#include <iostream>
#include <vector>
#include <boost/test/unit_test.hpp>

struct Cpp11Notification
{
	Cpp11Notification()
	{
		std::cout << "C++11 features enabled. Version: " << __cplusplus << std::endl;
	}
};

//! Testing move semantics with an object which can only be moved, but not
//! copied.
struct NonCopyableObject : boost::noncopyable
{
	NonCopyableObject(const std::string& msg) : msg(msg) {}
	NonCopyableObject(NonCopyableObject&& o) : msg(std::move(o.msg)) {}
	NonCopyableObject& operator = (NonCopyableObject&& o){ msg = std::move(o.msg); return *this; }
	std::string msg;
};

//! Testing move semantics: Creates a 'NonCopyableObject' and returns it by
//! moving it back.
NonCopyableObject createNonCopyableObject()
{
	return std::move(NonCopyableObject("Testing move semantics"));
}

BOOST_GLOBAL_FIXTURE (Cpp11Notification)

BOOST_AUTO_TEST_SUITE(Cpp11LanguageTests)

BOOST_AUTO_TEST_CASE (testAuto)
{
	std::vector<int> v;
	v.push_back(5);
	auto it = v.begin();
	std::cout << "C++11 auto test: " << *it << std::endl;
}

BOOST_AUTO_TEST_CASE (testLambda)
{
	std::vector<int> v;
	v.push_back(5);
	std::for_each(v.begin(), v.end(), [&](int& i) { return i=i*i; });
}

BOOST_AUTO_TEST_CASE (testMoveSemantics)
{
	std::vector<NonCopyableObject> v;
	v.emplace_back(createNonCopyableObject());
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

BOOST_AUTO_TEST_SUITE(Cpp11LibraryTests)

BOOST_AUTO_TEST_CASE (testUniquePtr)
{
	// Create a unique_ptr
	std::unique_ptr<int> i(new int(1337));
	std::cout << "unique_ptr i: " << *i << std::endl;
	
	// Now try transfer-of-ownership to new pointer
	std::unique_ptr<int> j = std::move(i);
	std::cout << "unique_ptr j: " << *j << std::endl;
	
	// Old pointer is useless.
	BOOST_CHECK(i.get() == nullptr);
	
	// New pointer contains value
	BOOST_CHECK_EQUAL(*j, 1337);
}

BOOST_AUTO_TEST_SUITE_END()
