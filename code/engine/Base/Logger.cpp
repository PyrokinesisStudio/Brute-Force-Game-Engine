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

#include "Logger.h"

#ifdef _MSC_VER
	#pragma warning( disable: 4512 )
#endif

#include <iostream>
#include <fstream>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/log/attributes/clock.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/sinks/basic_sink_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace BFG {
namespace Base {
namespace Logger {

void setupFileSink(const std::string& Filename);
void setupCoutSink();
void setupCore(SeverityLevel minLogLevel);

std::ostream& operator<< (std::ostream& strm, const SeverityLevel& svl)
{
	switch (svl)
	{
		case SL_DEBUG:
			return strm << "debug";
		case SL_INFORMATION:
			return strm << "info";
		case SL_WARNING:
			return strm << "warn";
		case SL_ERROR:
			return strm << "error";
	}
}

void Init(SeverityLevel Min_Log_Level, const std::string& Filename)
{
#ifndef NDEBUG
	static bool already_initialized = false;
	
	assert("It seems that you called Base::Logger::Init() twice."
	       " This is not recommended." && ! already_initialized);

	already_initialized = true;
#endif
	
	setupCore(Min_Log_Level);
	setupFileSink(Filename);
	setupCoutSink();
}

void setupFileSink(const std::string& Filename)
{
	if (Filename.empty())
		return;

	typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
	boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
	
	sink->locked_backend()->add_stream
	(
		boost::make_shared<std::ofstream>(Filename.c_str())
	);

	logging::add_common_attributes();

	sink->set_formatter
	(
		expr::format("%1% %2% %3%\t%4%")
			% expr::attr<unsigned int>("LineID")
			% expr::attr<boost::posix_time::ptime>("TimeStamp")
			% expr::attr<SeverityLevel>("Severity")
			% expr::smessage
	);
	
	sink->locked_backend()->auto_flush(true);
		
	logging::core::get()->add_sink(sink);
}

void setupCoutSink()
{
	typedef sinks::synchronous_sink<sinks::text_ostream_backend> TextSinkT;
	boost::shared_ptr<TextSinkT> pSink = boost::make_shared<TextSinkT>();

	boost::shared_ptr<std::ostream> pOutStream
	(
		&std::cout,
		logging::empty_deleter()
	);

	pSink->locked_backend()->add_stream(pOutStream);
	logging::core::get()->add_sink(pSink);
}

void setupCore(SeverityLevel minLogLevel)
{
	boost::shared_ptr<logging::core> pCore = logging::core::get();

	pCore->add_global_attribute
	(
		"TimeStamp",
		attrs::local_clock()
	);

	pCore->set_filter
	(
		expr::attr<SeverityLevel>("Severity") >= minLogLevel
	);
}

} // Logger
} // Base

//BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(bfg_log, BFG::Base::Logger::SourceT);
BOOST_LOG_GLOBAL_LOGGER_DEFAULT(bfg_log, BFG::Base::Logger::SourceT)

} // namespace BFG

