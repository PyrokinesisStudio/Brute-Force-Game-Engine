
#include <Core/String.h>


#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(StringTest)

using namespace BFG;

BOOST_AUTO_TEST_CASE(testSplit)
{
    std::string token;
    std::string rest;
    std::string source = "first second third fourth";
    bool res = false;
    
    res = split(source, " ", token, rest);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(token, "first");
}

BOOST_AUTO_TEST_CASE(testSplit2)
{
    std::string token;
    std::string rest;
    std::string source = "first-*-second-*-third-*-fourth";
    bool res = false;
    
    res = split(source, "-*-", token, rest);
    BOOST_CHECK_EQUAL(res, true);
    BOOST_CHECK_EQUAL(token, "first");
}

BOOST_AUTO_TEST_CASE(testSplitNegative)
{
    std::string token;
    std::string rest;
    std::string source = "firstSecond";
    bool res = true;
    
    res = split(source, " ", token, rest);
    BOOST_CHECK_EQUAL(res, false);
    BOOST_CHECK_EQUAL(token, "");
}

BOOST_AUTO_TEST_CASE(testSplitAll)
{
    std::string source = "first second third fourth";
    std::vector<std::string> tokens;
    tokenize(source, " ", tokens);
    BOOST_CHECK_EQUAL(tokens.empty(), false);
    BOOST_CHECK_EQUAL(tokens[0], "first");
    BOOST_CHECK_EQUAL(tokens[1], "second");
    BOOST_CHECK_EQUAL(tokens[2], "third");
    BOOST_CHECK_EQUAL(tokens[3], "fourth");
}

BOOST_AUTO_TEST_CASE(testSplitAll2)
{
    std::string source = "first-*-second-*-third-*-fourth";
    std::vector<std::string> tokens;
    tokenize(source, "-\*-", tokens);
    BOOST_CHECK_EQUAL(tokens.empty(), false);
    BOOST_CHECK_EQUAL(tokens[0], "first");
    BOOST_CHECK_EQUAL(tokens[1], "second");
    BOOST_CHECK_EQUAL(tokens[2], "third");
    BOOST_CHECK_EQUAL(tokens[3], "fourth");
}

BOOST_AUTO_TEST_SUITE_END()