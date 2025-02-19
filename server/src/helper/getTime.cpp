#include "../../include/webserv.hpp"

long long getCurrentTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long microseconds = static_cast<long long>(tv.tv_sec) * 1000000 + tv.tv_usec;
	return microseconds;
}
