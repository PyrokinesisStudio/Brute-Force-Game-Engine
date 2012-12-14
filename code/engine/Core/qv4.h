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

#ifndef BFG_QV4_H
#define BFG_QV4_H

#include <ostream>
#include <boost/geometry/extensions/arithmetic/cross_product.hpp>

#include <Core/Defs.h>
#include <Core/v3.h>
#include <Core/Types.h>
#include <Core/XmlTree.h>

namespace BFG {

template <typename T>
class quaternion4
{
public:
	quaternion4(T w = 1, T x = 0, T y = 0, T z = 0) :
	w(w),
	x(x),
	y(y),
	z(z)
	{
	}

	const T* ptr() const
	{
		return &w;
	}

	T* ptr()
	{
		return &w;
	}
	
	vector3<T> operator* (const vector3<T>& v) const
	{
		// nVidia SDK implementation
		vector3<T> uv, uuv;
		vector3<T> qvec(x, y, z);
		uv = boost::geometry::cross_product(qvec, v);
		uuv = boost::geometry::cross_product(qvec, uv);
		uv *= (2.0f * w);
		uuv *= 2.0f;

		return v + uv + uuv;
	}

	quaternion4<T>& operator = (const quaternion4<T>& rhs)
	{
		w = rhs.w;
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;	
	}

	quaternion4 operator + (const quaternion4& rhs) const
	{
		return quaternion4(w+rhs.w,x+rhs.x,y+rhs.y,z+rhs.z);
	}

	quaternion4 operator - (const quaternion4& rhs) const
	{
		return quaternion4(w-rhs.w,x-rhs.x,y-rhs.y,z-rhs.z);
	}

	quaternion4 operator * (const quaternion4& rhs) const
	{
		// NOTE:  Multiplication is not generally commutative, so in most
		// cases p*q != q*p.

		return quaternion4
		(
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,
			w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
			w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x
		);
	}

	quaternion4 operator * (T scalar) const
	{
		return quaternion4(scalar*w, scalar*x, scalar*y, scalar*z);
	}

	friend quaternion4 operator * (T scalar, const quaternion4& rhs)
	{
		return quaternion4
		(	
			scalar*rhs.w,
			scalar*rhs.x,
			scalar*rhs.y,
			scalar*rhs.z
		);
	}

	quaternion4 operator - () const
	{
		return quaternion4(-w, -x, -y, -z);
	}

	vector3<T> xAxis(void) const
	{
		//T fTx  = 2*x;
		T fTy  = 2*y;
		T fTz  = 2*z;
		T fTwy = fTy*w;
		T fTwz = fTz*w;
		T fTxy = fTy*x;
		T fTxz = fTz*x;
		T fTyy = fTy*y;
		T fTzz = fTz*z;

		return vector3<T>(1-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);
	}

	vector3<T> yAxis(void) const
	{
		T fTx  = 2*x;
		T fTy  = 2*y;
		T fTz  = 2*z;
		T fTwx = fTx*w;
		T fTwz = fTz*w;
		T fTxx = fTx*x;
		T fTxy = fTy*x;
		T fTyz = fTz*y;
		T fTzz = fTz*z;

		return vector3<T>(fTxy-fTwz, 1-(fTxx+fTzz), fTyz+fTwx);
	}

	vector3<T> zAxis(void) const
	{
		T fTx  = 2*x;
		T fTy  = 2*y;
		T fTz  = 2*z;
		T fTwx = fTx*w;
		T fTwy = fTy*w;
		T fTxx = fTx*x;
		T fTxz = fTz*x;
		T fTyy = fTy*y;
		T fTyz = fTz*y;

		return vector3<T>(fTxz+fTwy, fTyz-fTwx, 1-(fTxx+fTyy));
	}

	T getRoll(bool reprojectAxis = true) const
	{
		if (reprojectAxis)
		{
			// roll = atan2(localx.y, localx.x)
			// pick parts of xAxis() implementation that we need
			T fTx  = 2*x;
			T fTy  = 2*y;
			T fTz  = 2*z;
			T fTwz = fTz*w;
			T fTxy = fTy*x;
			T fTyy = fTy*y;
			T fTzz = fTz*z;

			// Vector3(1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);

			return T(std::atan2(fTxy+fTwz, T(1-(fTyy+fTzz))));
		}
		else
		{
			return T(std::atan2(2*(x*y + w*z), w*w + x*x - y*y - z*z));
		}
	}

	T getPitch(bool reprojectAxis = true) const
	{
		if (reprojectAxis)
		{
			// pitch = atan2(localy.z, localy.y)
			// pick parts of yAxis() implementation that we need
			T fTx  = 2*x;
//			T fTy  = 2*y;
			T fTz  = 2*z;
			T fTwx = fTx*w;
			T fTxx = fTx*x;
			T fTyz = fTz*y;
			T fTzz = fTz*z;

			// Vector3(fTxy-fTwz, 1-(fTxx+fTzz), fTyz+fTwx);
			return T(std::atan2(fTyz+fTwx, T(1-(fTxx+fTzz))));
		}
		else
		{
			// internal version
			return T(std::atan2(2*(y*z + w*x), w*w - x*x - y*y + z*z));
		}
	}

	T getYaw(bool reprojectAxis = true) const
	{
		if (reprojectAxis)
		{
			// yaw = atan2(localz.x, localz.z)
			// pick parts of zAxis() implementation that we need
			T fTx  = 2*x;
			T fTy  = 2*y;
			T fTz  = 2*z;
			T fTwy = fTy*w;
			T fTxx = fTx*x;
			T fTxz = fTz*x;
			T fTyy = fTy*y;

			// Vector3(fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy));
			return T(std::atan2(fTxz+fTwy, T(1-(fTxx+fTyy))));
		}
		else
		{
			// internal version
			return T(std::asin(-2*(x*z - w*y)));
		}
	}

	T w, x, y, z;

	static const quaternion4 ZERO;
	static const quaternion4 IDENTITY;

};

template <typename T> const quaternion4<T> quaternion4<T>::ZERO(0.0,0.0,0.0,0.0);
template <typename T> const quaternion4<T> quaternion4<T>::IDENTITY(1.0,0.0,0.0,0.0);

typedef quaternion4<f32> qv4;

template <typename T>
std::ostream& operator << (std::ostream& o, const quaternion4<T>& q)
{
	o << q.w << ", " << q.x << ", " << q.y << ", " << q.z;
	return o;
}

//! Converts four separated values within a string into a qv4. Examples:
//! \see stringToVector3
//! "41.0 .63, 614.5, 6.512"           // Ok!
//! "0.0001, 1521.9f, 0.001f 141.09"   // Ok!
//! \exception std::runtime_error If the string couldn't be parsed
BFG_CORE_API void stringToQuaternion4(const std::string& input, qv4& output);

BFG_CORE_API qv4 loadQuaternion(XmlTreeT tree);

} // namespace BFG

#endif
