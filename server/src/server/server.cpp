#include "../../include/webserv.hpp"

void coreLoop(std::map<int, ConfigData> &data) {
	while (running) {
		for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
			ConfigData &configData = it->second;
			int numEvents = epoll_wait(configData.epollFd, configData.events, MAX_EVENTS, 1);
			if (numEvents == -1) {
				Logger::error("Failed to wait for events.");
				break;
			}
			for (int i = 0; i < numEvents; ++i) {
				if (configData.events[i].data.fd == configData.serverFd) {
					addEvent(configData);
				} else if (configData.events[i].events & EPOLLIN) {
					epollIn(configData, i);
				} else if (configData.events[i].events & EPOLLOUT) {
					epollOut(configData, i);
				} else if (configData.events[i].events & EPOLLHUP) {
					epollHup(configData, i);
				}
			}
		}
	}
}

void startServer(std::map<int, ConfigData> &data) {

	if(!initServerConfigTmp(data)) {
		Logger::error("Server init doesn't finished successfully");
		closeEverything(data);
		return ;
	}
	Logger::info("Server init finished successfully");

	if(!initLocations(data)) {
		closeEverything(data);
		Logger::error("Locations init doesn't finished successfully");
		return ;
	}

	coreLoop(data);

	closeEverything(data);
}
