#include "../../../include/webserv.hpp"

void epollHup(ConfigData &configData, int i) {
	bool tmp = false;

	for (size_t j = 0; j < configData.servers.size(); ++j) {
	ServerContext &srv = configData.servers[j].serverContex;
		for (std::map<int, int>::iterator it = srv.fds.begin(); it != srv.fds.end(); ++it) {
			if (it->second == configData.events[i].data.fd) {
				int clientFd = it->first;
				HttpResponse &res = srv.responses[clientFd];
				epoll_ctl(configData.epollFd, EPOLL_CTL_DEL, configData.events[i].data.fd, NULL);
				close(configData.events[i].data.fd);
				srv.fds.erase(clientFd);
				srv.pids.erase(clientFd);
				res.readFromCgiFinished = true;
				res.state = SENDING_HEADERS;
				tmp = true;
				parseCgiContent(res);
				break;
			}
		}

		if (tmp) {
			break;
		}
	}
	if (!tmp) {
		std::map<int, HttpRequest>::iterator itReqData = configData.requests.begin();

		for (; itReqData != configData.requests.end(); itReqData++) {
			int clientFd = itReqData->second.clientFd;
			if (clientFd == configData.events[i].data.fd) {
				for (size_t i = 0; i < configData.servers.size(); i++) {
					ServerContext &ctx = configData.servers[i].serverContex;
					if (ctx.requests.find(clientFd) != ctx.requests.end()) {
						ctx.requests.erase(clientFd);
					}
				}
				epoll_ctl(configData.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
				configData.requests.erase(clientFd);
				close(clientFd);
			}
			Logger::info("Epoll Hup called for req with clientFd %i", clientFd);
			break;
		}
	}
}
