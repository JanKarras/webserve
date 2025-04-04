#include "../../include/webserv.hpp"

void handleRequest(int clientFd, ConfigData &configData) {

	size_t index = 0;

	std::map<int, ConfigData> data;

	data[configData.port] = configData;

	for (size_t i = 0; i < configData.servers.size(); i++) {
		server &server = configData.servers[i];
		ServerContext &serverContext = server.serverContex;
		if (serverContext.requests.find(clientFd) != serverContext.requests.end()) {
			index = i;
			Logger::info("Request was found for server: %s index = %i", server.server_name.c_str(), index);
		}
	}
	server &Server = configData.servers[index];

	HttpRequest req = Server.serverContex.requests[clientFd];
	HttpResponse &res = Server.serverContex.responses[clientFd];

	location *foundLocation = NULL;
	std::string &path = req.path;

	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];

		if (path.find(loc.name) == 0) {
			foundLocation = &loc;
			break;
		}

		for (size_t j = 0; j < loc.files.size(); j++) {
			std::string filePath = loc.files[j].path;

			if (filePath.find(Server.root) == 0) {
				filePath = filePath.substr(Server.root.size());
			}

			if (path == filePath) {
				foundLocation = &loc;
				break;
			}
		}

		if (foundLocation) {
			break;
		}
	}

	if (foundLocation) {
		if (req.method == GET) {
			if (foundLocation->get == false) {
				Logger::error("Method GET not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("GET in location %s called", foundLocation->name.c_str());
				routeRequestGET(req, res, Server);
			}
		} else if (req.method == POST) {
			if (foundLocation->post == false) {
				Logger::error("Method POST not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("POST in location %s called", foundLocation->name.c_str());
			}
		} else if (req.method == DELETE) {
			if (foundLocation->del == false) {
				Logger::error("Method DELETE not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("DELETE in location %s called", foundLocation->name.c_str());
			}
		} else {
			Logger::error("Unknown method: %i", req.method);
			handle501(res);
		}
	} else {
		Logger::error("Unknown location for path", req.path.c_str());
		handle404(res);
	}

	handleHome(res);

	res.version = req.version;
	res.state = SENDING_HEADERS;


	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = clientFd;

	if (epoll_ctl(configData.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
		std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
		close(clientFd);
		Server.serverContex.requests.erase(clientFd);
		Server.serverContex.responses.erase(clientFd);
		return;
	}
	res.startTime = getCurrentTime();
}
