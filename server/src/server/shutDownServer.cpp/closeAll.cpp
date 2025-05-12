#include "../../../include/webserv.hpp"

void closeHttpRequests(std::map<int, HttpRequest *> &requests, int epollFd) {
	std::map<int, HttpRequest *>::iterator it = requests.begin();
	for (; it != requests.end(); it++) {
		int clientFd = it->second->clientFd;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		close(clientFd);
	}
}

void closeHttpResponse(std::map<int, HttpResponse> &responses, int epollFd) {
	std::map<int, HttpResponse>::iterator it = responses.begin();
	for (; it != responses.end(); it++) {
		int clientFd = it->first;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		close(clientFd);
	}
}

void closeFds(std::map<int, int> &fds, int epollFd) {
	std::map<int, int>::iterator it = fds.begin();
	for (; it != fds.end(); it++) {
		int cgiOutFd = it->second;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, cgiOutFd, NULL);
		close(cgiOutFd);
	}
}

void closecgiFds(std::map<int, int> &cgifds, int epollFd) {
	std::map<int, int>::iterator it = cgifds.begin();
	for (; it != cgifds.end(); it++) {
		int cgiInFds = it->second;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, cgiInFds, NULL);
		close(cgiInFds);
	}
}

void killChildProcesses(std::map<int, pid_t> &pids) {
	for (std::map<int, pid_t>::iterator it = pids.begin(); it != pids.end(); ++it) {
		pid_t childPid = it->second;

		if (kill(childPid, SIGTERM) == -1) {
			Logger::error("Failed to send SIGTERM to child PID %d", childPid);
		} else {
			Logger::info("Sent SIGTERM to child PID %d", childPid);
		}
		int status = 0;
		waitpid(childPid, &status, 0);
	}
	pids.clear();
}

void closeEverything(std::map<int, ConfigData> &data) {
	std::map<int, ConfigData>::iterator it = data.begin();
	for (; it != data.end(); it++) {
		ConfigData &config = it->second;
		int serverFd = config.serverFd;
		int epollFd = config.epollFd;
		for (size_t i = 0; i < config.servers.size(); i++) {
			server &srv = config.servers[i];
			ServerContext &srvCon = srv.serverContex;
			killChildProcesses(srvCon.pids);
			closeFds(srvCon.fds, epollFd);
			closecgiFds(srvCon.cgifds, epollFd);
			closeHttpResponse(srvCon.responses, epollFd);
			closeHttpRequests(srvCon.requests, epollFd);
		}
		epoll_ctl(epollFd, EPOLL_CTL_DEL, serverFd, NULL);
		close(serverFd);
		close(epollFd);
	}
}
