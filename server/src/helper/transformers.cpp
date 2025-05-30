#include "../../include/webserv.hpp"

std::string toStringInt(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::string toString(long long number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

int toIntString(const std::string &str) {
    std::stringstream ss(str);
    int number;
    ss >> number;
    return number;
}

std::string sizeTToHex(size_t number) {
	std::stringstream ss;
	ss << std::hex << number;
	return ss.str();
}
