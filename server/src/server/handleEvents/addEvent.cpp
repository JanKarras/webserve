#include "../../../include/webserv.hpp"

bool addEvent(ConfigData &configData) {
	struct sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);

	int clientFd = accept(configData.serverFd, (struct sockaddr*)&clientAddress, &clientAddressLength);
	if (clientFd == -1) {
		std::cerr << "Failed to accept client connection." << std::endl;
		return (false);
	}
	if (setNonBlocking(clientFd)) {
		std::cerr << "Failed to set client socket non-blocking: " << strerror(errno) << std::endl;
		close(clientFd);
		return (false);
	}

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLHUP;
	event.data.fd = clientFd;
	if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
		std::cerr << "Failed to add client socket to epoll instance: " << strerror(errno) << std::endl;
		close(clientFd);
		return (false);
	}
	configData.requests[clientFd] = HttpRequest();
	configData.requests[clientFd].startTime = getCurrentTime();
	configData.requests[clientFd].clientFd = clientFd;
	Logger::info("client : %i connection added", clientFd);
	return (true);
}
