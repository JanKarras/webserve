#include "../../../include/webserv.hpp"

void epollIn(ConfigData &configData, int i) {
	bool tmp = false;

	for (size_t j = 0; j < configData.servers.size(); j++) {
		ServerContext &srv = configData.servers[j].serverContex;
		for (std::map<int, int>::iterator it = srv.fds.begin(); it != srv.fds.end(); it++) {
			if (it->second == configData.events[i].data.fd) {
				handleCgiRead(configData, i, srv);
				tmp = true;
				break;
			}
		}
		if (tmp) {
			break;
		}
	}

	if (tmp) {
		return ;
	} else {
		handleEventReq(configData, i);
	}
}
