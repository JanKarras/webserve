/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sigint.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 12:14:43 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/18 13:51:45 by jkarras          ###   ########.fr       */
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

	if (serverContext->requests.empty()) {
		std::cout << "No active clients to close." << std::endl;
	} else {
		for (std::map<int, HttpRequest>::iterator it = serverContext->requests.begin(); it != serverContext->requests.end(); ++it) {
			close(it->first);
			std::cout << "client " << it->first << " closed." << std::endl;
		}
	}

	

	exit(0);
}
