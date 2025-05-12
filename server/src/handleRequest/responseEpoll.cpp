#include "../../include/webserv.hpp"

void closeCgiConnection(server &Server, int clientFd, ConfigData &configData, HttpResponse &res, int state) {
	if (Server.serverContex.pids.find(clientFd) != Server.serverContex.pids.end()) {
		kill(Server.serverContex.pids[clientFd], SIGKILL);
		waitpid(Server.serverContex.pids[clientFd], NULL, 0);
		Server.serverContex.pids.erase(clientFd);
	}

	if (Server.serverContex.cgifds.find(clientFd) != Server.serverContex.cgifds.end()) {
		if (state == 1 && state == 2) {
			epoll_ctl(configData.epollFd, EPOLL_CTL_DEL, Server.serverContex.cgifds[clientFd], NULL);
		}
		close(Server.serverContex.cgifds[clientFd]);
		Server.serverContex.cgifds.erase(clientFd);
	}

	if (Server.serverContex.fds.find(clientFd) != Server.serverContex.fds.end()) {
		if (state == 2) {
			epoll_ctl(configData.epollFd, EPOLL_CTL_DEL, Server.serverContex.fds[clientFd], NULL);
		}
		close(Server.serverContex.fds[clientFd]);
		Server.serverContex.fds.erase(clientFd);
	}
}

bool setCgiEpoll(server &Server, int clientFd, ConfigData &configData, HttpResponse &res) {

	if (Server.serverContex.cgifds.find(clientFd) != Server.serverContex.cgifds.end()) {
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = Server.serverContex.cgifds[clientFd];
		if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, Server.serverContex.cgifds[clientFd], &event) == -1) {
			std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
			closeCgiConnection(Server, clientFd, configData, res, 0);
			return true;
		}
	}

	if (Server.serverContex.fds.find(clientFd) != Server.serverContex.fds.end()) {
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLHUP;
		event.data.fd = Server.serverContex.fds[clientFd];
		if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, Server.serverContex.fds[clientFd], &event) == -1) {
			std::cerr << "Failed to modify epoll event for EPOLLIN." << std::endl;
			closeCgiConnection(Server, clientFd, configData, res, 1);
			return true;
		}
		res.state = NOT_STARTED;
	}
	return (false);
}

void addResponseEpoll(server &Server, int clientFd, ConfigData &configData, HttpResponse &res) {
	if (setCgiEpoll(Server, clientFd, configData, res)) {
		handle500(res);
	}

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = clientFd;

	if (epoll_ctl(configData.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
		std::cerr << "Failed to modify epoll event for EPOLLOUT respons." << std::endl;
		closeCgiConnection(Server, clientFd, configData, res, 2);
		epoll_ctl(configData.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		close(clientFd);
		Server.serverContex.requests.erase(clientFd);
		Server.serverContex.responses.erase(clientFd);
		return;
	}
}
