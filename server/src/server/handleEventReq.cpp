#include "../../include/webserv.hpp"

bool handleEventReq(ConfigData &configData, int i) {
	char buffer[BUFFER_SIZE];
	std::string data;
	int bytesRead = recv(configData.events[i].data.fd, buffer, sizeof(buffer) - 1, 0);
	std::ofstream outFile("output.txt", std::ios::app);
	if (!outFile) {
        std::cerr << "Error opening file for writing.\n";
        return 1;
    }

    outFile.write(buffer, bytesRead);

    if (outFile.fail()) {
        std::cerr << "Error writing to file.\n";
        return 1;
    }

    outFile.close();
	if (bytesRead == -1) {
		std::cerr << "Failed to read from client.\n";
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

