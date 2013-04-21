#include <boost/lambda/lambda.hpp>
#include <Base/Logger.h>

#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(LoggerTestSuite)

BOOST_AUTO_TEST_CASE (testInit)
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "LoggerTest.log");
}

BOOST_AUTO_TEST_CASE (testSeverities)
{
	dbglog << "This is dbglog!";
	infolog << "I am infolog.";
	warnlog << "Attention, warnlog!";
	errlog << "errlog reporting in";
}

BOOST_AUTO_TEST_SUITE_END()
