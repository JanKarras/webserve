#include "../header/Logger.hpp"

std::string getTimestamp() {
	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);
	std::ostringstream oss;
	oss << (now->tm_year + 1900) << "-"
		<< ((now->tm_mon + 1) < 10 ? "0" : "") << (now->tm_mon + 1) << "-"
		<< (now->tm_mday < 10 ? "0" : "") << now->tm_mday << "T"
		<< (now->tm_hour < 10 ? "0" : "") << now->tm_hour << ":"
		<< (now->tm_min < 10 ? "0" : "") << now->tm_min << ":"
		<< (now->tm_sec < 10 ? "0" : "") << now->tm_sec << "Z";
	return oss.str();
}

void Logger::info(const char* message, ...) {

	std::cout << "[" << getTimestamp() << "] [INFO] ";

	va_list args;
	va_start(args, message);

	vprintf(message, args);
	std::cout << std::endl;

	va_end(args);
}

void Logger::debug(const char* message, ...) {

	std::cout << "[" << getTimestamp() << "] [DEBUG] ";

	va_list args;
	va_start(args, message);

	vprintf(message, args);
	std::cout << std::endl;

	va_end(args);
}

void Logger::error(const char* message, ...) {

	std::cout << "[" << getTimestamp() << "] [ERROR] ";

	va_list args;
	va_start(args, message);

	vprintf(message, args);
	std::cout << std::endl;

	va_end(args);
}

void Logger::warn(const char* message, ...) {

	std::cout << "[" << getTimestamp() << "] [WARNING] ";

	va_list args;
	va_start(args, message);

	vprintf(message, args);
	std::cout << std::endl;

	va_end(args);
}
