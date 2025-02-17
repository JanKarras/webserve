#ifndef WEBSERV_HPP

#define WEBSERV_HPP

#include <string>
#include <iostream>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../classes/header/ConficData.hpp"

#define MAX_EVENTS 1024

bool parseConfic(std::string path, ConficData *data);


#endif
