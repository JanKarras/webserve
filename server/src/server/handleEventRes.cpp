#include "../../include/webserv.hpp"

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
        Logger::error("Error: No matching server found for clientFd %d", clientFd);
        return false;
    }


    HttpResponse &response = serverContext->responses.at(clientFd);


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

        if (serverContext->fds.find(clientFd) != serverContext->fds.end()) {
            char buffer[CHUNK_SIZE];
            int readPipeFd = serverContext->fds[clientFd];
            long long time = getCurrentTime();
            if (time - response.startTime > TIME_TO_KILL_CHILD * 1000000) {
                Logger::debug("Timeout exceeded for clientFd: %d, killing process.", clientFd);
                kill(serverContext->pids[clientFd], SIGTERM);
                close(readPipeFd);
                close(clientFd);
                serverContext->fds.erase(clientFd);
                serverContext->requests.erase(clientFd);
                serverContext->responses.erase(clientFd);
                return false;
            }
            int bytesRead = read(readPipeFd, buffer, CHUNK_SIZE);
            if (bytesRead > 0) {
                Logger::debug("Read %d bytes from pipe for clientFd: %d", bytesRead, clientFd);
                ssize_t bytesSent = send(clientFd, buffer, bytesRead, 0);
                if (bytesSent == -1) {
                    Logger::debug("Error sending chunk to clientFd: %d", clientFd);
                    close(readPipeFd);
                    close(clientFd);
                    serverContext->fds.erase(clientFd);
                    serverContext->requests.erase(clientFd);
                    serverContext->responses.erase(clientFd);
                    return false;
                }
                return true;
            } else if (bytesRead == 0) {
                Logger::debug("Pipe closed, finishing response for clientFd: %d", clientFd);
                close(readPipeFd);
                serverContext->fds.erase(clientFd);
                response.state = RESP_COMPLETE;
                epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
                close(clientFd);
                serverContext->requests.erase(clientFd);
                serverContext->responses.erase(clientFd);
                return false;
            } else {
                Logger::debug("Error reading from pipe for clientFd: %d", clientFd);
                close(readPipeFd);
                close(clientFd);
                serverContext->fds.erase(clientFd);
                serverContext->requests.erase(clientFd);
                serverContext->responses.erase(clientFd);
                return false;
            }
        } else {
            size_t bodySize = response.body.size();
            size_t offset = response.bodySent;
            size_t chunkSize = CHUNK_SIZE;
            while (offset < bodySize) {
                size_t bytesToSend = std::min(chunkSize, bodySize - offset);
                ssize_t bytesSent = send(clientFd, response.body.c_str() + offset, bytesToSend, 0);
                if (bytesSent == -1) {
                    Logger::error("Error sending body to clientFd: %d", clientFd);
                    close(clientFd);
                    serverContext->requests.erase(clientFd);
                    serverContext->responses.erase(clientFd);
                    return false;
                }
                offset += bytesSent;
                response.bodySent = offset;
                if (offset < bodySize) {
                    return true;
                }
            }
            response.state = RESP_COMPLETE;
            epoll_ctl(data.epollFd, EPOLL_CTL_DEL, clientFd, NULL);
            close(clientFd);
            serverContext->requests.erase(clientFd);
            serverContext->responses.erase(clientFd);
        }
    }
    return true;
}
