/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleEventReq.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:43:38 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/19 12:43:46 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

bool handleEventReq(ServerContext &ServerContext, struct epoll_event *events, int i) {
	char buffer[BUFFER_SIZE];
	std::string data;
	int bytesRead = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead == -1) {
		std::cerr << "Failed to read from client.\n";
		return (false);
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
	return (true);
}
