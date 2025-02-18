/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 12:16:29 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/18 14:18:58 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void handleRequest(int clientFd, ServerContext &ServerContext) {

	printHttpRequest(ServerContext.requests[clientFd]);

	HttpRequest req = ServerContext.requests[clientFd];
	HttpResponse &res = ServerContext.responses[clientFd];

	res.version = req.version;
	res.statusCode = 200;
	res.statusMessage = "OK";
	res.body = "<html><body><h1>Welcome to the server!</h1></body></html>";
	res.headers["Content-Type"] = "text/html";

	res.state = SENDING_HEADERS;

	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = clientFd;

	if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
		std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
		close(clientFd);
		ServerContext.requests.erase(clientFd);
		ServerContext.responses.erase(clientFd);
		return;
	}
	res.startTime = getCurrentTime();
}
