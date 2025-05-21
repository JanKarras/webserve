#include "../../include/webserv.hpp"

void handleErrorRequest(int clientFd, ConfigData &data, HttpRequest &req) {

	ServerContext *serverContext = &data.servers[0].serverContex;

	//Logger::info("Error request detected for clientFd %i, error code %i", clientFd, req.exitStatus);

	HttpResponse &res = serverContext->responses[clientFd];


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
		res.statusMessage = "Method not allowed";
		filePath = "public/error/405.html";
		res.state = SENDING_HEADERS;
		break;
	case HTTP_SERVER_ERROR:
		res.statusMessage = "Http internl server error";
		filePath = "public/error/500.html";
		res.state = SENDING_HEADERS;
		break;
	case HTTP_ENTITY_TOO_LARGE:
		res.statusMessage = "Payload Too Large";
		filePath = "public/error/413.html";
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

	if (epoll_ctl(data.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
		std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
		epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		close(clientFd);
		serverContext->requests.erase(clientFd);
		serverContext->responses.erase(clientFd);
		return;
	}

	res.startTime = getCurrentTime();
}
