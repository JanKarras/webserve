#include "../../include/webserv.hpp"

void startServer(ConficData &conficData, bool conficFlag) {
	ServerContext ServerContext;
	ServerContext.serverFd = -1;
	ServerContext.epollFd = -1;

	struct sockaddr_in serverAddress;
	struct epoll_event event, events[MAX_EVENTS];
	if (conficFlag) {
		if (!initServerConfic(ServerContext, serverAddress, event, conficData)) {
			return ;
		}
	} else {
		if (!initServer(ServerContext, serverAddress, event)) {
			return ;
		}
	}

	initRoutes(ServerContext);

	while (running) {
		int numEvents = epoll_wait(ServerContext.epollFd, events, MAX_EVENTS, -1);
		if (numEvents == -1) {
			std::cerr << "Failed to wait for events." << std::endl;
			break;
		}
		for (int i = 0; i < numEvents; ++i) {
			if (events[i].data.fd == ServerContext.serverFd) {
				if (!addEvent(ServerContext, event)) {
					continue;
				}
			} else if (events[i].events & EPOLLIN) {
				if (!handleEventReq(ServerContext, events, i)) {
					continue;
				}
			} else if (events[i].events & EPOLLOUT) {
				if (!handleEventRes(ServerContext, events, i)) {
					continue;
				}
			}
		}
	}

	std::cout << "test\n";

	closeAll(ServerContext);
}

