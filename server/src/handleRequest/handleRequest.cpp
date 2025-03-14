#include "../../include/webserv.hpp"

void handleRequest(int clientFd, ConfigData &configData) {

	size_t index = 0;

	for (size_t i = 0; i < configData.servers.size(); i++) {
		server &server = configData.servers[i];
		ServerContext &serverContext = server.serverContex;
		if (serverContext.requests.find(clientFd) != serverContext.requests.end()) {
			index = i;
			Logger::info("Request was found for server: %s", server.server_name);
		}
	}


	HttpRequest req = configData.servers[index].serverContex.requests[clientFd];
	HttpResponse &res = configData.servers[index].serverContex.responses[clientFd];

	// if (req.method == GET) {
	// 	if(req.cgi && ServerContext.cgi.find(req.path) != ServerContext.cgi.end()) {
	// 		routeRequestCGI(req, res, ServerContext, clientFd);
	// 	} else {
	// 		routeRequestGET(req, res, ServerContext);
	// 	}
	// } else if (req.method == POST) {
	// 	routeRequestPOST(req, res, ServerContext);
	// } else if (req.method == DELETE) {
	// 	routeRequestDELETE(req, res, ServerContext);
	// } else {
	// 	handle405(res);
	// }

	res.version = req.version;
	res.state = SENDING_HEADERS;

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = clientFd;

	// if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
	// 	std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
	// 	close(clientFd);
	// 	ServerContext.requests.erase(clientFd);
	// 	ServerContext.responses.erase(clientFd);
	// 	return;
	// }
	res.startTime = getCurrentTime();
}
