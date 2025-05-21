#include "../../../include/webserv.hpp"

void sendErrorHandle(int clientFd, ConfigData &data, ServerContext *serverContext) {
	epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
	close(clientFd);
	serverContext->requests.erase(clientFd);
	serverContext->responses.erase(clientFd);
}

void sendHeader(HttpResponse &response, ServerContext *serverContext, ConfigData &data, int clientFd) {
	std::string headers;
	if (response.body.size() > CHUNK_SIZE) {
		response.headers["Transfer-Encoding"] = "chunked";
		response.headers.erase("Content-Length");
		response.chunked = true;
	}
	headers.append(response.version);
	headers.append(" ");
	headers.append(toString(response.statusCode));
	headers.append(" ");
	headers.append(response.statusMessage);
	headers.append("\r\n");
	for (std::map<std::string, std::string>::iterator it = response.headers.begin(); it != response.headers.end(); ++it) {
		headers.append(it->first);
		headers.append(": ");
		headers.append(it->second);
		headers.append("\r\n");
	}
	headers.append("\r\n");

	//redirectOutfile(headers, "./debugFiles/output_response.txt", headers.size(), headers.size());

	ssize_t bytesSent = send(clientFd, headers.c_str(), headers.size(), 0);
	if (bytesSent == -1) {
		Logger::error("Error sending headers to clientFd: %d", clientFd);
		sendErrorHandle(clientFd, data, serverContext);
		return ;
	}
	response.state = SENDING_BODY;
}

void sendBodyCunkend(HttpResponse &response, ServerContext *serverContext, ConfigData &data, int clientFd) {
	if (response.body.size() != 0) {
		size_t newBodySize = response.body.size();
		if (response.body.size() > CHUNK_SIZE) {
			newBodySize = CHUNK_SIZE;
		}
		std::string sendString;
		sendString.append(sizeTToHex(newBodySize));
		sendString.append("\r\n");
		sendString.append(response.body.substr(0, newBodySize));
		sendString.append("\r\n");
		response.body.erase(0, newBodySize);
		//redirectOutfile(sendString, "./debugFiles/output_response.txt", 100, sendString.size());
		ssize_t bytesSent = send(clientFd, sendString.c_str(), sendString.size(), 0);
		if (bytesSent == -1) {
			Logger::error("Error sending body to clientFd: %d", clientFd);
			sendErrorHandle(clientFd, data, serverContext);
			return ;
		}
		if (response.body.size() != 0) {
			return ;
		}
	}

	ssize_t bytesSend = send(clientFd, "0\r\n\r\n", 5, 0);
	if (bytesSend == -1) {
		Logger::error("Error sending body to clientFd: %d", clientFd);
		sendErrorHandle(clientFd, data, serverContext);
		return ;
	}
	response.state = RESP_COMPLETE;
	epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
	close(clientFd);
	serverContext->requests.erase(clientFd);
	serverContext->responses.erase(clientFd);
	Logger::info("client : %i connection closed", clientFd);
}

void sendBody(HttpResponse &response, ServerContext *serverContext, ConfigData &data, int clientFd) {
	ssize_t bytesSent = send(clientFd, response.body.c_str(), response.body.size(), 0);

	if (bytesSent == -1) {
		Logger::error("Error sending body to clientFd: %d", clientFd);
		sendErrorHandle(clientFd, data, serverContext);
		return ;
	}

	response.state = RESP_COMPLETE;
	epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
	close(clientFd);
	serverContext->requests.erase(clientFd);
	serverContext->responses.erase(clientFd);
	Logger::info("client : %i connection closed", clientFd);
}

bool handleEventRes(ConfigData &data, int i) {
	int clientFd = data.events[i].data.fd;
	ServerContext *serverContext = NULL;

	for (size_t j = 0; j < data.servers.size(); j++) {
		if (data.servers[j].serverContex.responses.find(clientFd) != data.servers[j].serverContex.responses.end()) {
			serverContext = &data.servers[j].serverContex;
			break;
		}
	}

	if (!serverContext) {
		return false;
	}

	HttpResponse &response = serverContext->responses.at(clientFd);

	if (response.state == SENDING_HEADERS) {
		sendHeader(response, serverContext, data, clientFd);
	} else if (response.state == SENDING_BODY) {
			if (response.chunked) {
				sendBodyCunkend(response, serverContext, data, clientFd);
			} else {
				sendBody(response, serverContext, data, clientFd);
			}
	}
	return true;
}
