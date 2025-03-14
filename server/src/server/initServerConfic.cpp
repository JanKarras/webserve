#include "../../include/webserv.hpp"

bool initServerConfic(ServerContext &ServerContext, struct sockaddr_in &serverAddress, struct epoll_event &event, ConfigData &conficData) {
	(void) ServerContext;
	(void) serverAddress;
	(void) event;
	(void) conficData;
	return (false);
	// conficData.servers.clear();
	// std::cout << "confic\n";
	// return (false);
	// ServerContext.serverFd = socket(AF_INET, SOCK_STREAM, 0);
	// if (ServerContext.serverFd == -1) {
	// 	std::cerr << "Failed to create socket." << std::endl;
	// 	return false;
	// }

	// serverAddress.sin_family = AF_INET;
	// serverAddress.sin_addr.s_addr = INADDR_ANY;
	// serverAddress.sin_port = htons(PORT);


	// if (bind(ServerContext.serverFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
	// 	std::cerr << "Failed to bind socket." << std::endl;
	// 	close(ServerContext.serverFd);
	// 	return false;
	// }

	// if (listen(ServerContext.serverFd, MAX_CLIENTS) == -1) {
	// 	std::cerr << "Failed to listen." << std::endl;
	// 	close(ServerContext.serverFd);
	// 	return false;
	// }

	// ServerContext.epollFd = epoll_create1(0);
	// if (ServerContext.epollFd == -1) {
	// 	std::cerr << "Failed to create epoll instance." << std::endl;
	// 	close(ServerContext.serverFd);
	// 	return false;
	// }

	// event.events = EPOLLIN;
	// event.data.fd = ServerContext.serverFd;
	// if (epoll_ctl(ServerContext.epollFd, EPOLL_CTL_ADD, ServerContext.serverFd, &event) == -1) {
	// 	std::cerr << "Failed to add server socket to epoll instance." << std::endl;
	// 	close(ServerContext.serverFd);
	// 	close(ServerContext.epollFd);
	// 	return false;
	// }

	// std::cout << "Server started. Listening on port " << PORT << std::endl;
	// return (true);
}
