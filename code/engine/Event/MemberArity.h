/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

#ifndef BFG_EVENT_MEMBERARITY_H
#define BFG_EVENT_MEMBERARITY_H

namespace BFG {
namespace Event { 

template <typename Class> struct member_arity {};

template <typename Class, typename R>
struct member_arity<R (Class::*) (void)>
{
	typedef R result_type;
	static const int arity = 0;
	static const bool c = false;
	typedef void arg1_type;
	typedef void arg2_type;
};

template <typename Class, typename R>
struct member_arity<R (Class::*) (void) const>
{
	typedef R result_type;
	static const int arity = 0;
	static const bool c = true;
	typedef void arg1_type;
	typedef void arg2_type;
};

template <typename Class, typename R, typename T1>
struct member_arity<R (Class::*) (T1)>
{
	typedef R result_type;
	static const int arity = 1;
	static const bool c = false;
	typedef T1 arg1_type;
	typedef void arg2_type;
};

template <typename Class, typename R, typename T1>
struct member_arity<R (Class::*) (T1) const>
{
	typedef R result_type;
	static const int arity = 1;
	static const bool c = true;
	typedef T1 arg1_type;
	typedef void arg2_type;
};

template <typename Class, typename R, typename T1, typename T2>
struct member_arity<R (Class::*) (T1, T2)>
{
	typedef R result_type;
	static const int arity = 2;
	static const bool c = false;
	typedef T1 arg1_type;
	typedef T2 arg2_type;
};

template <typename Class, typename R, typename T1, typename T2>
struct member_arity<R (Class::*) (T1, T2) const>
{
	typedef R result_type;
	static const int arity = 2;
	static const bool c = true;
	typedef T1 arg1_type;
	typedef T2 arg2_type;
};

} // namespace Event
} // namespace BFG

#endif
