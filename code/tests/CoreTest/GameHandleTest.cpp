#include <boost/test/unit_test.hpp>
#include <Core/GameHandle.h>

BOOST_AUTO_TEST_SUITE(GameHandleTestSuite)

BOOST_AUTO_TEST_CASE (testNormalHandleNotEqualNetworkHandle)
{
	BFG::GameHandle normalHandle = BFG::generateHandle();
	BFG::GameHandle networkHandle = BFG::generateNetworkHandle();

	BOOST_CHECK(normalHandle > 0);
	BOOST_CHECK(networkHandle > 0);

	BOOST_CHECK_NE(normalHandle, networkHandle);
}

BOOST_AUTO_TEST_CASE (testCountingUp)
{
	BFG::GameHandle handle1 = BFG::generateHandle();
	BFG::GameHandle handle2 = BFG::generateHandle();
	
	BOOST_CHECK_NE(handle1, handle2);
}

BOOST_AUTO_TEST_CASE (testSerialization)
{
	BFG::GameHandle orgHandle = BFG::generateHandle();
	std::string strHandle = BFG::stringify(orgHandle);
	BFG::GameHandle handle = BFG::destringify(strHandle);
	BOOST_CHECK_EQUAL(orgHandle, handle);
}


BOOST_AUTO_TEST_SUITE_END()
