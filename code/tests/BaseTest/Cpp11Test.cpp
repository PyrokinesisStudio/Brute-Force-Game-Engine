#if defined(__GXX_EXPERIMENTAL_CXX0X__) || _MSC_VER > 1500

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

BOOST_AUTO_TEST_CASE (testNullPtr)
{
	int* p = nullptr;
	p = p;
}

BOOST_AUTO_TEST_CASE (testStaticAssert)
{
	static_assert(true != false, "Something seriously wrong here");
}

BOOST_AUTO_TEST_SUITE_END()

#endif
