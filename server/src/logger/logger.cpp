#include "../../include/webserv.hpp"

void logger(std::string message, int code) {
	if (code == 0) {
		std::cout << getCurrentTime() << " [DEBUG] " << message << std::endl;
	} else if (code == 1) {
		std::cout << getCurrentTime() << " [INFO] " << message << std::endl;
	} else if (code == 2) {
		std::cout << getCurrentTime() << " [WARNING] " << message << std::endl;
	} else if (code == 3) {
		std::cout << getCurrentTime() << " [ERROR] " << message << std::endl;
	}
}
