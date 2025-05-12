#include "../../include/webserv.hpp"

server &getServer(int clientFd, ConfigData &configData) {
	for (size_t i = 0; i < configData.servers.size(); i++) {
		server &server = configData.servers[i];
		ServerContext &serverContext = server.serverContex;
		if (serverContext.requests.find(clientFd) != serverContext.requests.end()) {
			Logger::info("Request was found for server: %s", server.server_name.c_str());
			return (server);
		}
	}
	return (configData.servers[0]);
}

HttpRequest getReq(server &Server, int clientFd) {
	std::map<int, HttpRequest *>::iterator it = Server.serverContex.requests.find(clientFd);

	if (it != Server.serverContex.requests.end()) {
		return(*it->second);
	} else {
		Logger::error("Request not found for clientFd: %i on server: %s", clientFd, Server.server_name.c_str());
		HttpRequest req;
		req.method = INVALID;
		return(req);
	}
}

void normelaizePaths(HttpRequest &req, server &Server) {
	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];
		if (!loc.regularLocation && loc.name.length() > 1 && loc.name[loc.name.length() - 1] == '/') {
			if (req.path + "/" == loc.name) {
				Logger::debug("Normalizing path: %s => %s", req.path.c_str(), (req.path + "/").c_str());
				req.path += "/";
				break;
			}
		}
	}
}
