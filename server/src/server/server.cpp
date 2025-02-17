/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jkarras <jkarras@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 12:16:18 by jkarras           #+#    #+#             */
/*   Updated: 2025/02/17 14:05:59 by jkarras          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/webserv.hpp"

void startServerWithConfic(ConficData &data) {

}

void startServer(void) {
	int serverFd;
	int epollFd;

	struct sockaddr_in serverAddress;
    struct epoll_event event, events[MAX_EVENTS];

	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1) {
		std::cerr << "Failed to create socket." << std::endl;
		return ;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(8080);

	if (bind(serverFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "Failed to bind socket." << std::endl;
		close(serverFd);
		return ;
	}
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
