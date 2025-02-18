#ifndef WEBSERV_HPP

#define WEBSERV_HPP

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>

#include "../classes/header/ConficData.hpp"

#define MAX_EVENTS 1024
#define MAX_CLIENTS 1024
#define PORT 8080

struct ServerContext {
	int serverFd;
	int epollFd;
};

bool parseConfic(std::string path, ConficData *data);


#endif
