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

#ifndef BFG_CHAR_ARRAY_H__
#define BFG_CHAR_ARRAY_H__

#include <algorithm>
#include <string>
#include <boost/array.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_pointer.hpp>

typedef boost::array<char,128> CharArray128T;
typedef boost::array<char,512> CharArray512T;

template <int ArraySize>
boost::array<char, ArraySize> stringToArray(const std::string& s)
{
	boost::array<char, ArraySize> a;
	std::fill(a.begin(), a.end(), 0);
	std::copy(s.begin(), s.end(), a.begin());
	return a;
}

template <typename T, typename ArrayT>
void arrayToValue(T& val, const ArrayT& array, size_t offset, typename boost::disable_if<boost::is_pointer<T> >::type* /*dummy*/ = 0)
{
	memcpy(&val, &array[offset], sizeof(T));
}

template <typename T, typename ArrayT>
void valueToArray(const T& val, ArrayT& array, const size_t offset)
{
	assert( sizeof(T) <= array.size() - offset );
	memcpy(array.data() + offset, &val, sizeof(T));
}

#endif
