#include "../../include/webserv.hpp"

void handleErrorRequest(int clientFd, ServerContext &ServerContext) {

	HttpRequest req = ServerContext.requests[clientFd];
	HttpResponse &res = ServerContext.responses[clientFd];


	res.statusCode = req.exitStatus;

	res.version = req.version;
	res.state = SENDING_HEADERS;

	std::string filePath;

	switch (res.statusCode)
	{
	case HTTP_BAD_REQUEST:
		res.statusMessage = "Http bad request";
		filePath = "public/error/400.html";
		res.state = SENDING_HEADERS;
		break;
	case HTTP_FORBIDDEN:
		res.statusMessage = "Http frobidden";
		filePath = "public/error/403.html";
		res.state = SENDING_HEADERS;
		break;
	case HTTP_NOT_FOUND:
		res.statusMessage = "Http not found";
		filePath = "public/error/404.html";
		res.state = SENDING_HEADERS;
		break;
	case HTTP_METHOD_N_ALLOWED:
		res.statusMessage = "Http method not allowed";
		filePath = "public/error/405.html";
		res.state = SENDING_HEADERS;
		break;
	case HTTP_SERVER_ERROR:
		res.statusMessage = "Http internl server error";
		filePath = "public/error/500.html";
		res.state = SENDING_HEADERS;
		break;
	default:
		res.statusMessage = "Http not found";
		filePath = "public/error/404.html";
		res.state = SENDING_HEADERS;
		break;
	}

	res.body = getFileContent(filePath);

	if (res.body.size() == 0) {
		res.body = "<html><body><h1>" + res.statusMessage + "</h1></body></html>";
	}
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toStringInt(res.body.size());

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
