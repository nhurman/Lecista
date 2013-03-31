#pragma once

#include <sstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#define LOG_DEBUG(x) LOG("Debug ", x)
#ifdef DEBUG
	#define FILE (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)
	#define LOG(level, msg) Logger::get(level) << FILE << ":" << __LINE__ << "] " << msg << std::endl
#else
	#define LOG(level, msg)
#endif


namespace Lecista {

struct Logger
{
	static std::ostream& get(char const* msg)
	{
		std::ostream& stream = std::cerr;
		//stream.imbue(std::locale(std::cout.getloc(), new boost::posix_time::time_facet("[%H:%M:%S, ")));
		//stream << boost::posix_time::second_clock::local_time();
		stream << msg << "[";

		return stream;
	}
};

}
