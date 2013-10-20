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

#ifndef BASE_LOGGER_H
#define BASE_LOGGER_H

#ifdef _MSC_VER	// only for MSVC
  #pragma warning( disable: 4714 ) // force inline not inline
  #pragma warning( push )
  #pragma warning( disable: 4512 4244 4996 4100 )
#endif // _MSC_VER

// Fix for compile error within Boost.Log (operator problem) on OSX and Clang++
// See http://stackoverflow.com/questions/6573436/cant-compile-against-boost-log-on-os-x
#define BOOST_LOG_NO_UNSPECIFIED_BOOL

#include <string>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

namespace BFG {
namespace Base {
namespace Logger {

//! There's really no need for EnumGen here. ;-)
enum SeverityLevel
{
	SL_DEBUG       = boost::log::trivial::debug,
	SL_INFORMATION = boost::log::trivial::info,
	SL_WARNING     = boost::log::trivial::warning,
	SL_ERROR       = boost::log::trivial::error
};

//! To make sererity levels human readable.
std::ostream& operator<< (std::ostream& strm, const SeverityLevel& svl);

typedef boost::log::sources::severity_logger_mt
<
	Base::Logger::SeverityLevel
> SourceT;

/**
    You'd better call this only once. An assertion will warn you.
    Could work when called twice, but not recommended.
    
    \param minLogLevel This is the minimum logging level. See enum
                       Logger::SeverityLevel for your choice.
                         
    \param Filename The file the log records will be written into.
                    If omitted or empty, no file will be created.
*/
void Init(SeverityLevel minLogLevel, const std::string& Filename = "");

// Convenience macros (example of use can be found in the base test app)
#define dbglog  BOOST_LOG_SEV(BFG::bfg_log::get(), BFG::Base::Logger::SL_DEBUG)
#define infolog BOOST_LOG_SEV(BFG::bfg_log::get(), BFG::Base::Logger::SL_INFORMATION)
#define warnlog BOOST_LOG_SEV(BFG::bfg_log::get(), BFG::Base::Logger::SL_WARNING)
#define errlog  BOOST_LOG_SEV(BFG::bfg_log::get(), BFG::Base::Logger::SL_ERROR)

} // namespace Logger
} // namespace Base

BOOST_LOG_GLOBAL_LOGGER(bfg_log, BFG::Base::Logger::SourceT)

} // namespace BFG

#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

#endif 

