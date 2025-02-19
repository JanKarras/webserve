#include "../../include/webserv.hpp"

void closeAll(ServerContext ServerContext) {
	if (ServerContext.requests.empty()) {
		std::cout << "No active clients to close." << std::endl;
	} else {
		for (std::map<int, HttpRequest>::iterator it = ServerContext.requests.begin(); it != ServerContext.requests.end(); ++it) {
			close(it->first);
			std::cout << "client " << it->first << " closed." << std::endl;
		}
	}

	if (ServerContext.responses.empty()) {
		std::cout << "No active clients to close." << std::endl;
	} else {
		for (std::map<int, HttpResponse>::iterator it = ServerContext.responses.begin(); it != ServerContext.responses.end(); ++it) {
			close(it->first);
			std::cout << "client " << it->first << " closed." << std::endl;
		}
	}
	close(ServerContext.serverFd);
	close(ServerContext.epollFd);
}
