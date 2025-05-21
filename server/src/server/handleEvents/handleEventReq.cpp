#include "../../../include/webserv.hpp"

bool handleEventReq(ConfigData &configData, int i) {
	char buffer[BUFFER_SIZE];
	std::string data;

	int bytesRead = recv(configData.events[i].data.fd, buffer, sizeof(buffer), 0);

	//redirectOutfile(buffer, "./debugFiles/output.txt", 1000, bytesRead);

	if (bytesRead == -1) {
		return (false);
	} else if (bytesRead == 0) {
		close(configData.events[i].data.fd);
		configData.requests.erase(configData.events[i].data.fd);
		epoll_ctl(configData.epollFd, EPOLL_CTL_DEL, configData.events[i].data.fd, NULL);
	} else {
		data.append(buffer, bytesRead);
		parseHttpRequest(configData, configData.requests[configData.events[i].data.fd].clientFd, data);
		if (configData.requests[configData.events[i].data.fd].state == COMPLETE) {
			handleRequest(configData.events[i].data.fd, configData);
		} else if (configData.requests[configData.events[i].data.fd].state == ERROR) {
			handleErrorRequest(configData.events[i].data.fd, configData, configData.requests[configData.events[i].data.fd]);
		}
	}
	for (size_t i = 0; i < BUFFER_SIZE; i++) {
		buffer[i] = '\0';
	}

	return (true);
}

