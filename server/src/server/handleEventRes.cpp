#include "../../include/webserv.hpp"

bool handleEventRes(ServerContext &ServerContext, struct epoll_event *events, int i) {
	HttpResponse &response = ServerContext.responses[events[i].data.fd];
	if (response.state == SENDING_HEADERS) {
		std::string headers;
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
		ssize_t bytesSent = send(events[i].data.fd, headers.c_str(), headers.size(), 0);
		if (bytesSent == -1) {
			std::cerr << "Error sending headers to client." << std::endl;
			close(events[i].data.fd);
			ServerContext.requests.erase(events[i].data.fd);
			ServerContext.responses.erase(events[i].data.fd);
			return (false);
		}
		response.state = SENDING_BODY;
	} else if (response.state == SENDING_BODY) {
		if (ServerContext.fds.find(events[i].data.fd) != ServerContext.fds.end()) {
			char buffer[CHUNK_SIZE];
			int readPipeFd = ServerContext.fds[events[i].data.fd];
			long long time = getCurrentTime();
			if (time - response.startTime > 10000000) {
				kill(ServerContext.pids[events[i].data.fd], SIGTERM);
				close(readPipeFd);
				close(events[i].data.fd);
				ServerContext.fds.erase(events[i].data.fd);
				ServerContext.requests.erase(events[i].data.fd);
				ServerContext.responses.erase(events[i].data.fd);
				return false;
			}
			int bytesRead = read(readPipeFd, buffer, CHUNK_SIZE);
			if (bytesRead > 0) {
				ssize_t bytesSent = send(events[i].data.fd, buffer, bytesRead, 0);
				if (bytesSent == -1) {
					std::cerr << "Error sending chunk to client." << std::endl;
					close(readPipeFd);
					close(events[i].data.fd);
					ServerContext.fds.erase(events[i].data.fd);
					ServerContext.requests.erase(events[i].data.fd);
					ServerContext.responses.erase(events[i].data.fd);
					return false;
				}
				return true;
			} else if (bytesRead == 0) {
				close(readPipeFd);
				ServerContext.fds.erase(events[i].data.fd);
				response.state = RESP_COMPLETE;
				epoll_ctl(ServerContext.epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				close(events[i].data.fd);
				ServerContext.requests.erase(events[i].data.fd);
				ServerContext.responses.erase(events[i].data.fd);
				return false;
			} else if (bytesRead == -1) {
				return true;
			} else {
				std::cerr << "Error reading from pipe." << std::endl;
				close(readPipeFd);
				close(events[i].data.fd);
				ServerContext.fds.erase(events[i].data.fd);
				ServerContext.requests.erase(events[i].data.fd);
				ServerContext.responses.erase(events[i].data.fd);
				return false;
			}

		} else {
			size_t bodySize = response.body.size();
			size_t offset = response.bodySent;
			size_t chunkSize = CHUNK_SIZE;
			while (offset < bodySize) {
				size_t bytesToSend = std::min(chunkSize, bodySize - offset);
				ssize_t bytesSent = send(events[i].data.fd, response.body.c_str() + offset, bytesToSend, 0);
				if (bytesSent == -1) {
					std::cerr << "Error sending body to client." << std::endl;
					close(events[i].data.fd);
					ServerContext.requests.erase(events[i].data.fd);
					ServerContext.responses.erase(events[i].data.fd);
					return (false);
				}
				offset += bytesSent;
				response.bodySent = offset;
				if (offset < bodySize) {
					return (true);
				}
			}
			response.state = RESP_COMPLETE;
			epoll_ctl(ServerContext.epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
			close(events[i].data.fd);
			ServerContext.requests.erase(events[i].data.fd);
			ServerContext.responses.erase(events[i].data.fd);
		}
	}
	return (true);
}
