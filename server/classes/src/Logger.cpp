#include "../header/Logger.hpp"

const char* Logger::logLevelToString(LogLevel level) {
	switch (level)
	{
	case DEBUGGING_LOG:
		return "DEBUGGING";
		break;
	case INFO_LOG:
		return "INFO";
		break;
	case WARNING_LOG:
		return "WARNING";
		break;
	case ERROR_LOG:
		return "ERROR";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

void Logger::log(LogLevel level, const char* message, ...) {
	long long int time = getCurrentTime();

	std::cout << "[" << time << "] [" << logLevelToString(level) << "] ";

	va_list args;
	va_start(args, message);

	vprintf(message, args);
	std::cout << std::endl;

	va_end(args);
}
