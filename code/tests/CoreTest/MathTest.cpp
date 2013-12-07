#include <Core/Math.h>

#define BOOST_TEST_MODULE MathTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (distance)
{
	BFG::v3 a(-5.2f, 3.8f, 4.8f);
	BFG::v3 b(8.7f, -4.1f, 9.1f);

	BFG::f32 result = BFG::distance(a,b);
	BFG::f32 expected = 16.5563f;

	const BFG::f32 ERROR_MARGIN = 0.0001f;

	bool resultCorrect = std::fabs(result - expected) < ERROR_MARGIN;

	BOOST_REQUIRE (resultCorrect);
}

BOOST_AUTO_TEST_CASE (nearEnoughFloat)
{
	// Should be near enough (distance: 0)
	BFG::f32 a = 1.0f/10.0f;
	BFG::f32 b = a   *10.0f;
	bool result1 = BFG::nearEnough(b, 1.0f, BFG::EPSILON_F);

	// Should not be near enough (distance: 0.2f)
	BFG::f32 c = 1.2f;
	BFG::f32 d = 1.0f;
	bool result2 = BFG::nearEnough(c, d, BFG::EPSILON_F);

	BOOST_CHECK (result1);
	BOOST_CHECK (!result2);
}

BOOST_AUTO_TEST_CASE (nearEnough)
{
	// distance between `a' and `b' is 16.5563f
	BFG::v3 a(-5.2f, 3.8f, 4.8f);
	BFG::v3 b(8.7f, -4.1f, 9.1f);

	bool resultNull = BFG::nearEnough(a, b, 0.0f);
	bool resultLower = BFG::nearEnough(a, b, 16.5f);
	bool resultUpper = BFG::nearEnough(a, b, 16.6f);

	BOOST_CHECK (resultNull == false);
	BOOST_CHECK (resultUpper == true);
	BOOST_CHECK (resultLower == false);
}
