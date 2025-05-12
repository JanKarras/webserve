#include "../../../include/webserv.hpp"

void handleCgiRead(ConfigData &data, int i, ServerContext &srv) {
	int cgifd = data.events[i].data.fd;
	int clientFd = 0;


	for (std::map<int, int>::iterator it = srv.fds.begin(); it != srv.fds.end(); ++it) {
		if (it->second == cgifd) {
			clientFd = it->first;
		}
	}
	std::map<int, HttpResponse>::iterator itt = srv.responses.find(clientFd);
	HttpResponse &res = itt->second;

	char buffer[BUFFER_SIZE];

	ssize_t bytesRead = read(cgifd, buffer, CHUNK_SIZE);
	if (bytesRead == -1) {
		return;
	} else if (bytesRead == 0) {
		res.readFromCgiFinished = true;
		close(cgifd);
		srv.fds.erase(clientFd);
		res.state = SENDING_HEADERS;
	} else {
		res.body.append(buffer, bytesRead);
	}
}
