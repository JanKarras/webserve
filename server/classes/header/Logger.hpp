#ifndef LOGGER_HPP

#define LOGGER_HPP

#include "../../include/webserv.hpp"

class Logger {
	public:
		static void info(const char* message, ...);
		static void debug(const char* message, ...);
		static void error(const char* message, ...);
		static void warn(const char* message, ...);
};

#endif
