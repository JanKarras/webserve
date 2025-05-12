#include "../../../include/webserv.hpp"

void epollOut(ConfigData &configData, int i) {
	bool tmp = false;

	for (size_t j = 0; j < configData.servers.size(); j++) {
		ServerContext &srv = configData.servers[j].serverContex;
		for (std::map<int, int>::iterator it = srv.cgifds.begin(); it != srv.cgifds.end(); it++) {
			if (it->second == configData.events[i].data.fd) {
				handleCgiWrite(configData, i, srv);
				tmp = true;
				break;
			}
		}
		if (tmp) {
			break;
		}
	}

	if (tmp) {
		return;
	} else {
		handleEventRes(configData, i);
	}
}
