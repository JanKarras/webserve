#include "../../../include/webserv.hpp"

bool initServerConfigTmp(std::map<int, ConfigData> &data) {
	for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
		ConfigData &configData = it->second;

		configData.serverFd = socket(AF_INET, SOCK_STREAM, 0);
		if (configData.serverFd == -1) {
			Logger::error("Failed to create socket on port %i", configData.port);
			return (false);
		}

		int opt = 1;
		if (setsockopt(configData.serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			Logger::error("setsockopt(SO_REUSEADDR) failed");
			close(configData.serverFd);
			return (false);
		}

		configData.serverAddress.sin_family = AF_INET;
		configData.serverAddress.sin_addr.s_addr = INADDR_ANY;
		configData.serverAddress.sin_port = htons(configData.port);

		if (LOOP != 1) {
			if (bind(configData.serverFd, (struct sockaddr*)&configData.serverAddress, sizeof(configData.serverAddress)) == -1) {
				Logger::error("Failed to bind socket.");
				close(configData.serverFd);
				return (false);
			}
		} else {
			while (bind(configData.serverFd, (struct sockaddr*)&configData.serverAddress, sizeof(configData.serverAddress)) == -1) {
				Logger::error("bind failed, retry in 2 secs");
				sleep(2);
			}
		}

		if (listen(configData.serverFd, MAX_CLIENTS) == -1) {
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

		Logger::info("Server started and listening on port: %i", configData.port);
	}

	return true;
}
