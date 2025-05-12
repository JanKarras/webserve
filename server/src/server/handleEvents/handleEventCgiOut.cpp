#include "../../../include/webserv.hpp"

void handleCgiWrite(ConfigData &data, int i, ServerContext &srv) {
	int cgifd = data.events[i].data.fd;
	int clientFd= 0;
	for (std::map<int, int>::iterator it = srv.cgifds.begin(); it != srv.cgifds.end(); ++it) {
		if (it->second == cgifd) {
			clientFd = it->first;
		}
	}
	std::map<int, HttpResponse>::iterator itt = srv.responses.find(clientFd);
	HttpResponse &res = itt->second;

	size_t newBodySize = res.cgiBody.size();
	if (res.cgiBody.size() > CHUNK_SIZE) {
		newBodySize = CHUNK_SIZE;
	}
	std::string sendString = res.cgiBody.substr(0, newBodySize);
	ssize_t bytesSent = write(cgifd, sendString.c_str(), newBodySize);

	if (bytesSent == -1) {
		return;
	} else {
		res.cgiBody.erase(0, bytesSent);
	}
	if (res.cgiBody.size() != 0) {
		return;
	}
	Logger::debug("All body data sent to CGI for clientFd: %d", clientFd);
	epoll_ctl(data.epollFd, EPOLL_CTL_DEL, data.events[i].data.fd, NULL);
	close(cgifd);
	srv.cgifds.erase(clientFd);
	res.sendingBodyToCgi = false;
}
