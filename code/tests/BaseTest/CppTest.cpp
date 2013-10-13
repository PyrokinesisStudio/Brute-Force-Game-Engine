#include <boost/assign/std/vector.hpp>

#include <Base/Cpp.h>

#include <boost/test/unit_test.hpp>

bool even(int i)
{
	return i%2 == 0;
}

BOOST_AUTO_TEST_SUITE(CppTestSuite)

BOOST_AUTO_TEST_CASE (testEraseIf)
{
	using namespace boost::assign;
	
	std::vector<int> v;
	v += 1,2,3,4,5,6;
	
	erase_if(v, even);

	std::vector<int> expected;
	expected += 1,3,5;
	
	BOOST_CHECK_EQUAL_COLLECTIONS(v.begin(), v.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END()
