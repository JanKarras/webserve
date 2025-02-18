/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 12:16:18 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/18 11:04:32 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void handle_sigint(int sig, siginfo_t *siginfo, void *context) {
    ServerContext* serverContext = reinterpret_cast<ServerContext*>(context);
	(void)sig;
	(void)siginfo;
    std::cout << "\nCTRL+C recived" << std::endl;

    if (serverContext->serverFd != -1) {
        close(serverContext->serverFd);
        std::cout << "Server Socket closed." << std::endl;
    }

    if (serverContext->epollFd != -1) {
        close(serverContext->epollFd);
        std::cout << "epoll Instanz closed." << std::endl;
    }
    exit(0);
}

void startServerWithConfic(ConficData &data) {
	data.nb = 0;
}

int setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "Failed to get file descriptor flags." << std::endl;
		return -1;
	}

	flags |= O_NONBLOCK;  // Setze den non-blocking Flag
	if (fcntl(fd, F_SETFL, flags) == -1) {
		std::cerr << "Failed to set file descriptor to non-blocking." << std::endl;
		return -1;
	}

	return 0;
}

void handleRequest(int clientFd, std::string req) {
	if (clientFd == -1) {
		return;
	}
	std::cout << "Received data: " << req << std::endl;
	close(clientFd);
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
				setNonBlocking(clientFd);
				event.events = EPOLLIN;
				event.data.fd = clientFd;
				if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
					std::cerr << "Failed to add client socket to epoll instance." << std::endl;
					close(clientFd);
					continue;
				}
			} else if (events[i].events & EPOLLIN) {
				char buffer[1024];
				int bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, 0);
				if (bytesRead == -1) {
					std::cerr << "Failed to read from client.\n";
					continue;
				} else if (bytesRead == 0) {
					close(events[i].data.fd);
				} else {
					buffer[bytesRead] = '\0';
					std::string req(buffer);
					handleRequest(events[i].data.fd, req);
				}
			}
		}
	}
	close(ServerContext.serverFd);
	close(ServerContext.epollFd);
}

int main(int argc, char **argv) {

	ConficData data;

	if (argc > 2) {
		std::cout << "Bad arg num\n";
		return (1);
	} else if (argc == 2) {
		if (parseConfic(std::string(argv[1]), &data)) {
			return (1);
		} else {
			startServerWithConfic(data);
		}
	} else {
		startServer();
	}
	return (0);
}
