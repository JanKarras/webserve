#include "../../include/webserv.hpp"

void closeAll(ServerContext ServerContext) {
	if (ServerContext.fds.empty()) {
		std::cout << "No active file deskriptors to close." << std::endl;
	} else {
		for (std::map<int, int>::iterator it = ServerContext.fds.begin(); it != ServerContext.fds.end(); ++it) {
			close(it->second);
			std::cout << "Fd: " << it->first << " closed." << std::endl;
		}
	}

	if (ServerContext.pids.empty()) {
		std::cout << "No active childprozesses  to kill." << std::endl;
	} else {
		for (std::map<int, int>::iterator it = ServerContext.pids.begin(); it != ServerContext.pids.end(); ++it) {
			kill(it->second, SIGTERM);
			std::cout << "Fd: " << it->first << " closed." << std::endl;
		}
	}

	if (ServerContext.requests.empty()) {
		std::cout << "No active request clients to close." << std::endl;
	} else {
		for (std::map<int, HttpRequest>::iterator it = ServerContext.requests.begin(); it != ServerContext.requests.end(); ++it) {
			close(it->first);
			std::cout << "client " << it->first << " closed." << std::endl;
		}
	}

	if (ServerContext.responses.empty()) {
		std::cout << "No active response clients to close." << std::endl;
	} else {
		for (std::map<int, HttpResponse>::iterator it = ServerContext.responses.begin(); it != ServerContext.responses.end(); ++it) {
			close(it->first);
			std::cout << "client " << it->first << " closed." << std::endl;
		}
	}



	close(ServerContext.serverFd);
	close(ServerContext.epollFd);
}
