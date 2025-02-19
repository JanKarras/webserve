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
