#include "../../include/webserv.hpp"

// void startServer(ConfigData &conficData, bool conficFlag) {
// 	ServerContext ServerContext;
// 	ServerContext.serverFd = -1;
// 	ServerContext.epollFd = -1;

// 	struct sockaddr_in serverAddress;
// 	struct epoll_event event, events[MAX_EVENTS];
// 	if (conficFlag) {
// 		if (!initServerConfic(ServerContext, serverAddress, event, conficData)) {
// 			return ;
// 		}
// 	} else {
// 		if (!initServer(ServerContext, serverAddress, event)) {
// 			return ;
// 		}
// 	}

// 	initRoutes(ServerContext);

// 	while (running) {
// 		int numEvents = epoll_wait(ServerContext.epollFd, events, MAX_EVENTS, -1);
// 		if (numEvents == -1) {
// 			std::cerr << "Failed to wait for events." << std::endl;
// 			break;
// 		}
// 		for (int i = 0; i < numEvents; ++i) {
// 			if (events[i].data.fd == ServerContext.serverFd) {
// 				if (!addEvent(ServerContext, event)) {
// 					continue;
// 				}
// 			} else if (events[i].events & EPOLLIN) {
// 				if (!handleEventReq(ServerContext, events, i)) {
// 					continue;
// 				}
// 			} else if (events[i].events & EPOLLOUT) {
// 				if (!handleEventRes(ServerContext, events, i)) {
// 					continue;
// 				}
// 			}
// 		}
// 	}

// 	std::cout << "test\n";

// 	closeAll(ServerContext);
// }

bool initServerConfigTmp(std::map<int, ConfigData> &data) {
	for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
		ConfigData &configData = it->second;

		configData.serverFd = socket(AF_INET, SOCK_STREAM, 0);
		if (configData.serverFd == -1) {
			Logger::error("Failed to create socket on port %i", configData.port);
			return (false);
		}

		configData.serverAddress.sin_family = AF_INET;
		configData.serverAddress.sin_addr.s_addr = INADDR_ANY;
		configData.serverAddress.sin_port = htons(configData.port);

		if (bind(configData.serverFd, (struct sockaddr*)&configData.serverAddress, sizeof(configData.serverAddress)) == -1) {
			Logger::error("Failed to bind socket.");
			close(configData.serverFd);
			return (false);
		}

		if (listen(configData.serverFd, MAX_CLIENTS) == -1){
			Logger::error("Failed to listen.");
			close(configData.serverFd);
			return (false);
		}

		configData.epollFd = epoll_create1(0);
		if (configData.epollFd == -1) {
			Logger::error("Failed to create epoll instance.");
			close(configData.serverFd);
			return (false);
		}

		configData.event.events = EPOLLIN;
		configData.event.data.fd = configData.serverFd;
		if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, configData.serverFd, &configData.event) == -1) {
			Logger::error("Failed to add server socket to epoll instance.");
			close(configData.serverFd);
			close(configData.epollFd);
			return (false);
		}

		Logger::info("Server started and liestening on port: %i", configData.port);
	}

	return true;
}

void startServer(std::map<int, ConfigData> &data) {

	if(!initServerConfigTmp(data)) {
		Logger::error("Server init doesn't finished successfully");
		return ;
	}
	Logger::info("Server init finished successfully");

	while (running) {
		for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
			ConfigData &configData = it->second;
			int numEvents = epoll_wait(configData.epollFd, configData.events, MAX_EVENTS, 1);
			if (numEvents == -1) {
				std::cerr << "Failed to wait for events." << std::endl;
				break;
			}
			for (int i = 0; i < numEvents; ++i) {
				if (configData.events[i].data.fd == configData.serverFd) {
					if (!addEvent(configData)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLIN) {
					if (!handleEventReq(configData, i)) {
						continue;
					}
				} else if (configData.events[i].events & EPOLLOUT) {
					// if (!handleEventRes(ServerContext, events, i)) {
					// 	continue;
					// }
				}
			}
		}
	}

}
