/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   addEvent.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:42:47 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/19 12:43:05 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

bool addEvent(ServerContext &ServerContext, struct epoll_event &event) {
	struct sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);
	int clientFd = accept(ServerContext.serverFd, (struct sockaddr*)&clientAddress, &clientAddressLength);
	if (clientFd == -1) {
		std::cerr << "Failed to accept client connection." << std::endl;
		return (false);
	}
	if (setNonBlocking(clientFd)) {
		close(clientFd);
		return (false);
	}

	event.events = EPOLLIN;
	event.data.fd = clientFd;
	if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1) {
		std::cerr << "Failed to add client socket to epoll instance." << std::endl;
		close(clientFd);
		return (false);
	}
	ServerContext.requests[clientFd] = HttpRequest();
	ServerContext.requests[clientFd].startTime = getCurrentTime();
	return (true);
}
