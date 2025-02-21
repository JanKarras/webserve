#include "../../include/webserv.hpp"

void handleRequest(int clientFd, ServerContext &ServerContext) {

	printHttpRequest(ServerContext.requests[clientFd]);

	HttpRequest req = ServerContext.requests[clientFd];
	HttpResponse &res = ServerContext.responses[clientFd];

	if (req.method == 0) {
		routeRequestGET(req, res);
	} else if (req.method == 1) {
		routeRequestPOST(req, res);
	} else if (req.method == 2) {
		routeRequestDELETE(req, res);
	}

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = clientFd;

	if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
		std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
		close(clientFd);
		ServerContext.requests.erase(clientFd);
		ServerContext.responses.erase(clientFd);
		return;
	}
	res.startTime = getCurrentTime();
}
