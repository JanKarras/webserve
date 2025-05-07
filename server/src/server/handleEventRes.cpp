#include "../../include/webserv.hpp"

void stopEvent(ServerContext &server, int clientFd) {
	if (server.requests.find(clientFd) != server.requests.end()) {
		Logger::info("Erased request with clientFd: %i", clientFd);
		server.requests.erase(clientFd);
	}
	if (server.responses.find(clientFd) != server.responses.end()) {
		Logger::info("Erased response with clientFd: %i", clientFd);
		server.responses.erase(clientFd);
	}
	if (server.fds.find(clientFd) != server.fds.end()) {
		Logger::info("Closed fd to read from cgi with clientFd: %i", clientFd);
		close(server.fds[clientFd]);
	}
	if (server.cgifds.find(clientFd) != server.cgifds.end()) {
		Logger::info("Closed fd to write to cgi with clientFd: %i", clientFd);
		close(server.cgifds[clientFd]);
	}
	if (server.pids.find(clientFd) != server.pids.end()) {
		Logger::info("Killed chiled  with clientFd: %i", clientFd);
		kill(server.pids[clientFd], SIGTERM);
	}
}

std::string sizeTToHex(size_t number) {
    std::stringstream ss;
    ss << std::hex << number; // in Hex schreiben
    return ss.str();          // als String zurückgeben
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
        //Logger::error("Error: No matching server found for clientFd %d", clientFd);
        return false;
    }

	HttpResponse &response = serverContext->responses.at(clientFd);
	// if (response.readFromCgiFinished == false) {
	// 	return false;
	// }

	if (response.state == SENDING_HEADERS) {
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
		std::ofstream outFile("output_response.txt", std::ios::app);
		if (!outFile) {
			std::cerr << "Error opening file for writing.\n";
			return 1;
		}

		outFile.write(headers.c_str(), headers.size());

		if (outFile.fail()) {
			std::cerr << "Error writing to file.\n";
			return 1;
		}

		outFile.close();
        ssize_t bytesSent = send(clientFd, headers.c_str(), headers.size(), 0);
        if (bytesSent == -1) {
            Logger::error("Error sending headers to clientFd: %d", clientFd);
            close(clientFd);
            serverContext->requests.erase(clientFd);
            serverContext->responses.erase(clientFd);
            return false;
        }
        response.state = SENDING_BODY;
    } else if (response.state == SENDING_BODY) {

        // if (serverContext->fds.find(clientFd) != serverContext->fds.end() && serverContext->responses[clientFd].writeError == false) {
        //     char buffer[CHUNK_SIZE];
        //     int readPipeFd = serverContext->fds[clientFd];
        //     long long time = getCurrentTime();
        //     if (time - response.startTime > TIME_TO_KILL_CHILD * 1000000) {
        //         Logger::debug("Timeout exceeded for clientFd: %d, killing process.", clientFd);
        //         kill(serverContext->pids[clientFd], SIGTERM);
        //         close(readPipeFd);
        //         close(clientFd);
        //         serverContext->fds.erase(clientFd);
        //         serverContext->requests.erase(clientFd);
        //         serverContext->responses.erase(clientFd);
        //         return false;
        //     }
        //     int bytesRead = read(readPipeFd, buffer, CHUNK_SIZE);
		// 	Logger::debug("bytesRead: %i, readPipeFd: %i, buffer: %s", bytesRead, readPipeFd, buffer);
        //     if (bytesRead > 0) {
        //         Logger::debug("Read %d bytes from pipe for clientFd: %d", bytesRead, clientFd);
        //         ssize_t bytesSent = send(clientFd, buffer, bytesRead, 0);
        //         if (bytesSent == -1) {
        //             Logger::debug("Error sending chunk to clientFd: %d", clientFd);
        //             close(readPipeFd);
        //             close(clientFd);
        //             serverContext->fds.erase(clientFd);
        //             serverContext->requests.erase(clientFd);
        //             serverContext->responses.erase(clientFd);
        //             return false;
        //         }
        //         return true;
        //     } else if (bytesRead == 0) {
        //         Logger::debug("Pipe closed, finishing response for clientFd: %d", clientFd);
        //         close(readPipeFd);
        //         serverContext->fds.erase(clientFd);
        //         response.state = RESP_COMPLETE;
        //         epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
        //         close(clientFd);
        //         serverContext->requests.erase(clientFd);
        //         serverContext->responses.erase(clientFd);
        //         return false;
        //     } else {
        //         Logger::debug("Error reading from pipe %i for clientFd: %d", serverContext->fds[clientFd], clientFd);
		// 		return (false);
		// 		close(readPipeFd);
        //         close(clientFd);
        //         serverContext->fds.erase(clientFd);
        //         serverContext->requests.erase(clientFd);
        //         serverContext->responses.erase(clientFd);
        //         return false;
        //     }
        //} else {
			if (response.chunked) {
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
					std::ofstream outFile("output_response.txt", std::ios::app);
					if (!outFile) {
						std::cerr << "Error opening file for writing.\n";
						return 1;
					}

					outFile.write(sendString.c_str(), 100);

					if (outFile.fail()) {
						std::cerr << "Error writing to file.\n";
						return 1;
					}

					outFile.close();
					ssize_t bytesSent = send(clientFd, sendString.c_str(), sendString.size(), 0);
					if (bytesSent == -1) {
								Logger::error("Error sending body to clientFd: %d", clientFd);
								close(clientFd);
								serverContext->requests.erase(clientFd);
								serverContext->responses.erase(clientFd);
								return false;
					}
					if (response.body.size() != 0) {
						return (false);
					}
				}
			} else {
				//std::cout << response.body;
				ssize_t bytesSent = send(clientFd, response.body.c_str(), response.body.size(), 0);
				if (bytesSent == -1) {
							Logger::error("Error sending body to clientFd: %d", clientFd);
							close(clientFd);
							serverContext->requests.erase(clientFd);
							serverContext->responses.erase(clientFd);
							return false;
				}
			}



            // size_t bodySize = response.body.size();
            // size_t offset = response.bodySent;
            // size_t chunkSize = CHUNK_SIZE;
            // while (offset < bodySize) {
            //     size_t bytesToSend = std::min(chunkSize, bodySize - offset);
			// 	std::string hexNumber = sizeTToHex(bytesToSend);
			// 	std::string data = response.body.c_str() + offset;
			// 	hexNumber.append("\r\n");
			// 	data.append("\r\n");
			// 	hexNumber.append(data);
			// 	//send(clientFd, hexNumber.c_str(), hexNumber.size(), 0);
            //     ssize_t bytesSent = send(clientFd, hexNumber.c_str(), sizeTToHex(bytesToSend).size() + 2 + bytesToSend + 2, 0) - sizeTToHex(bytesToSend).size() - 4;
            //     if (bytesSent == -1) {
            //         Logger::error("Error sending body to clientFd: %d", clientFd);
            //         close(clientFd);
            //         serverContext->requests.erase(clientFd);
            //         serverContext->responses.erase(clientFd);
            //         return false;
            //     }
            //     offset += bytesSent;
            //     response.bodySent = offset;
            //     if (offset < bodySize) {
            //         return true;
            //     }
            // }
			if (response.chunked) {
				send(clientFd, "0\r\n", 3, 0);
				send(clientFd, "\r\n", 2, 0);
			}
            response.state = RESP_COMPLETE;
            epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
            close(clientFd);
            serverContext->requests.erase(clientFd);
            serverContext->responses.erase(clientFd);
			Logger::info("client : %i connection closed. Size of Req: %i. Size of Res: %i", clientFd, serverContext->requests.size(), serverContext->responses.size());

			// std::map<int, HttpRequest *>::iterator it = serverContext->requests.begin();
			// std::map<int, HttpResponse>::iterator itt = serverContext->responses.begin();
			// for (size_t i = 0; it != serverContext->requests.end(); ++i) {
			// 	Logger::info("")
			// }

    }
    return true;
}
