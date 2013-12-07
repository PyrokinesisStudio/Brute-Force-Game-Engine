/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

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

/** @file

	\note
	A lot of these functions were taken from OGRE. The reason is, that
	we try to strictly separate the renderer from all other modules. You
	(hopefully) won't find any calls to OGRE within other modules than the
	View. However, vectors and quaternions are often used elsewhere, too.
	This is why we copied (almost 1:1) some of OGREs vector and quaternion
	functions, which you might recognize by their name. One difference is,
	that we made some of them non-member functions.
*/

#ifndef BFG_MATH_H
#define BFG_MATH_H

#include <Core/v3.h>
#include <Core/qv4.h>
#include <Core/Defs.h>

#ifndef M_PI
  #define M_PI 3.14159265358979323846f
#endif

//! \brief Multiply with with this value to convert from Degrees to RADs.
#define DEG2RAD (M_PI  / 180.0f)

//! \brief Multiply with with this value to convert from RADs to Degrees.
#define RAD2DEG (180.0f / M_PI)

namespace BFG {

//! \brief Clamps values into a given range.
//! \param[in] value Original value
//! \param[in] min   Lower limit
//! \param[in] max   Upper limit
//! \return Clamped result
template<typename T>
T clamp(T value, T min, T max)
{
	if (value > max)
		return max;
	if (value < min)
		return min;
	else
		return value;
}

//! \brief Gets the sign of a value as coefficient
//! \return -1.0f if x<0 and 1.0f if x>0
template <typename T>
T sign(T x)
{
	return static_cast<f32>((x > 0) - (x < 0));
}

void BFG_CORE_API ceil(v3& lhs, const v3& rhs);
void BFG_CORE_API norm(v3& vec);
void BFG_CORE_API norm(qv4& quat);

//! \brief Calculates a quaternion from an euler angle and a rotation axis.
//! This function was copied from OGRE to reduce dependencies.
//! \param[out] result Storage for new Quaternion
//! \param[in] rfAngle Euler angle representing the rotation in RAD
//! \param[in] rkAxis A vector indicating around which axis shall be rotated
void BFG_CORE_API fromAngleAxis(qv4& result,
                                const f32 rfAngle,
                                const v3& rkAxis);

//! \brief Calculates a quaternion from two vectors and an optional fallback axis.
//! This function was copied from OGRE to reduce dependencies.
//! \param[in] src Vector pointing to the source direction
//! \param[in] dest Vector pointing to the destination direction
//! \param[in] fallbackAxis Sometimes, the rotation track is ambigous.
//!                         In case of doubt, this axis will be used.
//! \return The resulting quaternion
qv4 BFG_CORE_API rotationTo(const v3& src,
                            const v3& dest,
                            const v3& fallbackAxis = v3::ZERO);

//! \brief Compares two quaternions
bool BFG_CORE_API equals(const qv4& lhs, const qv4& rhs, const f32 epsilon = EPSILON_F);

//! \brief Calculates the distance from zero to a given vector.
f32 BFG_CORE_API length(const v3& vec);

//! \brief Calculates the distance between two vectors
f32 BFG_CORE_API distance(const v3& first, const v3& second);

//! \brief Calculates the angle between two vectors
//! \param[in] src First vector
//! \param[in] dest Second vector
//! \return Result in RAD
f32 BFG_CORE_API angleBetween(const v3& src, const v3& dest);

//! \brief Calculates the angle between two quaternions
//! \param[in] src First quaternion
//! \param[in] dest Second quaternion
//! \return Result in RAD
f32 BFG_CORE_API angleBetween(const qv4& src, const qv4& dest);

//! \brief Clamp for vectors.
//! \return The result.
v3 BFG_CORE_API clamp(const v3& original, const v3& minVec, const v3& maxVec);

qv4 BFG_CORE_API unitInverse(const qv4& quat);

//! \brief Converts three euler angles to a quaternion
//! \param[in] euler Euler angles as pitch, roll, yaw in RAD
//! \return The resulting quaternion
qv4 BFG_CORE_API eulerToQuaternion(const v3& euler);

//! \return True, if the two positions are in reach of each other given the
//!         radius. This radius is basically used as distance between the both.
bool BFG_CORE_API nearEnough(const v3& position1,
                             const v3& position2,
                             f32 radius);

//! \return True, if the two values are in distance of each other.
bool BFG_CORE_API nearEnough(const f32 value1,
                             const f32 value2,
                             const f32 distance);

//! \brief Linear Quaternion interpolation
//! \param[in] q0 Quaternion at the beginning of the interpolation
//! \param[in] q1 Quaternion at the end of the interpolation
//! \param[in] h Interpolation parameter in the range from 0 to 1
//! \return The interpolated quaternion
qv4 BFG_CORE_API lerp(const qv4& q0, const qv4& q1, f32 h);

//! \brief Spherical Linear Quaternion interpolation
//! \param[in] q0 Quaternion at the beginning of the interpolation
//! \param[in] q1 Quaternion at the end of the interpolation
//! \param[in] h Interpolation parameter in the range from 0 to 1
//! \return The interpolated quaternion
qv4 BFG_CORE_API slerp(const qv4& q0, const qv4& q1, f32 h);

//! \brief Linear interpolation
//! \param[in] a The beginning of the interpolation
//! \param[in] b The end of the interpolation
//! \param[in] coefficient Parameter in the range from 0 to 1
//! \return The interpolated value
template <typename T, typename ScalarT>
T interpolate(const T& a, const T& b, const ScalarT coefficient)
{
	if (coefficient > 1.0f)
		return b;
	else
		return a + (b - a) * coefficient;
}

} // namespace BFG



#endif
