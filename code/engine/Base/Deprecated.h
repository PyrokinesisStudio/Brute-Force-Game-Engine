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

#ifndef BFG_BASE_DEPRECATED_H
#define BFG_BASE_DEPRECATED_H

#if _MSC_VER
	#define BFG_DEPRECATED_START(text) __declspec(deprecated(text))
	#define BFG_DEPRECATED_END
#elif __GNUC__
	#define BFG_DEPRECATED_START(text)
	#define BFG_DEPRECATED_END __attribute__((deprecated))
#else
	//! Helper macro for \ref BFG_DEPRECATED
	#define BFG_DEPRECATED_START
	//! Helper macro for \ref BFG_DEPRECATED
	#define BFG_DEPRECATED_END
	#pragma message("WARNING: You need to implement BFG_DEPRECATED for this compiler")
#endif

//! Marks a function as deprecated.
/**
	This will emit a compiler warning and generate a deprecated tag for doxygen.

	The following must be defined within the Doxyfile in order to generate
	deprecated tags:

	\code
	PREDEFINED = "BFG_DEPRECATED(text)=START_COMMENT \deprecated text END_COMMENT"
	\endcode

	Note that START_COMMENT and END_COMMENT must be replaced by C++ comment
	preprocessor instructions.

	Example of usage within C++ code:

	\code
	BFG_DEPRECATED("Use: Event::id()")
	void getId()
	{
		return id;
	}
	\endcode
*/
#define BFG_DEPRECATED(text) BFG_DEPRECATED_START(text)BFG_DEPRECATED_END

#endif
