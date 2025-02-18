/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 12:16:18 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/18 14:31:02 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

int setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Failed to get file descriptor flags." << std::endl;
		return -1;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		std::cerr << "Failed to set file descriptor to non-blocking." << std::endl;
		return -1;
	}
	return 0;
}

void startServer(void) {
	ServerContext ServerContext;
	ServerContext.serverFd = -1;
	ServerContext.epollFd = -1;

	struct sockaddr_in serverAddress;
	struct epoll_event event, events[MAX_EVENTS];

	ServerContext.serverFd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET ipv4 protcol -> SOCK_STREAM tcp
	if (ServerContext.serverFd == -1) {
		std::cerr << "Failed to create socket." << std::endl;
		return ;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(PORT);


	if (bind(ServerContext.serverFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "Failed to bind socket." << std::endl;
		close(ServerContext.serverFd);
		return ;
	}

	if (listen(ServerContext.serverFd, MAX_CLIENTS) == -1) {
		std::cerr << "Failed to listen." << std::endl;
		close(ServerContext.serverFd);
		return ;
	}

	ServerContext.epollFd = epoll_create1(0);
	if (ServerContext.epollFd == -1) {
		std::cerr << "Failed to create epoll instance." << std::endl;
		close(ServerContext.serverFd);
		return ;
	}

	event.events = EPOLLIN;
	event.data.fd = ServerContext.serverFd;
	if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_ADD, ServerContext.serverFd, &event) == -1) {
		std::cerr << "Failed to add server socket to epoll instance." << std::endl;
		close(ServerContext.serverFd);
		close(ServerContext.epollFd);
		return ;
	}

	std::cout << "Server started. Listening on port " << PORT << std::endl;

	struct sigaction sa;
	sa.sa_sigaction = handle_sigint;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		std::cerr << "SigInt error!" << std::endl;
		return ;
	}

	while (1) {
		int numEvents = epoll_wait(ServerContext.epollFd, events, MAX_EVENTS, -1);
		if (numEvents == -1) {
			std::cerr << "Failed to wait for events." << std::endl;
			break;
		}
		for (int i = 0; i < numEvents; ++i) {
			if (events[i].data.fd == ServerContext.serverFd) {
				struct sockaddr_in clientAddress;
				socklen_t clientAddressLength = sizeof(clientAddress);
				int clientFd = accept(ServerContext.serverFd, (struct sockaddr*)&clientAddress, &clientAddressLength);
				if (clientFd == -1) {
					std::cerr << "Failed to accept client connection." << std::endl;
					continue;
				}
				if (setNonBlocking(clientFd)) {
					close(clientFd);
					continue;
				}

				event.events = EPOLLIN;
				event.data.fd = clientFd;
				if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
					std::cerr << "Failed to add client socket to epoll instance." << std::endl;
					close(clientFd);
					continue;
				}
				ServerContext.requests[clientFd] = HttpRequest();
				ServerContext.requests[clientFd].startTime = getCurrentTime();
			} else if (events[i].events & EPOLLIN) {
				char buffer[BUFFER_SIZE];
				std::string data;
				int bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, 0);

				if (bytesRead == -1) {
					std::cerr << "Failed to read from client.\n";
					continue;
				} else if (bytesRead == 0) {
					close(events[i].data.fd);
					ServerContext.requests.erase(events[i].data.fd);
					epoll_ctl(ServerContext.epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				} else {
					data.append(buffer, bytesRead);
					parseHttpRequest(ServerContext.requests[events[i].data.fd], data);
					if (ServerContext.requests[events[i].data.fd].state == COMPLETE) {
						handleRequest(events[i].data.fd, ServerContext);
					} else if (ServerContext.requests[events[i].data.fd].state == ERROR) {
						handleErrorRequest(events[i].data.fd, ServerContext);
					}
				}
			} else if (events[i].events & EPOLLOUT) {
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
						continue;
					}
					response.state = SENDING_BODY;
				} else if (response.state == SENDING_BODY) {
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
							return;
						}
						offset += bytesSent;
						response.bodySent = offset;

						if (offset < bodySize) {
							continue;
						}
					}
					response.state = RESP_COMPLETE;
					epoll_ctl(ServerContext.epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
        			close(events[i].data.fd);
        			ServerContext.requests.erase(events[i].data.fd);
        			ServerContext.responses.erase(events[i].data.fd);
				}
			}
		}
	}
	close(ServerContext.serverFd);
	close(ServerContext.epollFd);
}

