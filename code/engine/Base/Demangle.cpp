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

#include <Base/Demangle.h>

#ifdef __GNUG__
	#include <cstdlib>
	#include <memory>
	#include <cxxabi.h>
#endif

namespace BFG {

#ifdef __GNUG__
static std::string demangleGnu(const char* name)
{
	int status = -4; // some arbitrary value to eliminate the compiler warning

	std::unique_ptr<char, void(*)(void*)> res
	{
		abi::__cxa_demangle(name, NULL, NULL, &status),
		std::free
	};

	return status == 0 ? res.get() : name;
}
#endif

#ifdef _WIN32
static std::string demangleWin(const char* name)
{
	// TODO: Win32
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms681400%28v=vs.85%29.aspx
	// https://github.com/ohtorii/demangle/blob/master/src/main.cpp#L12
	// http://stackoverflow.com/questions/13777681/demangling-in-msvc
	return name;
}
#endif

std::string demangle(const char* name)
{
#ifdef __GNUG__
	return demangleGnu(name);
#elif _WIN32
	return demangleWin(name);
#else
	return name;
#endif
}

} // namespace BFG

