#ifndef LOGGER_HPP

#define LOGGER_HPP

#include "../../include/webserv.hpp"

enum LogLevel {
	DEBUGGING_LOG,
	INFO_LOG,
	WARNING_LOG,
	ERROR_LOG
};

class Logger {
	public:
		static const char* logLevelToString(LogLevel level);
		static void log(LogLevel level, const char* message, ...);
};

#endif
