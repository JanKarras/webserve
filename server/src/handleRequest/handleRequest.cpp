#include "../../include/webserv.hpp"


void handleRequest(int clientFd, ConfigData &configData) {
	std::map<int, ConfigData> data;
	data[configData.port] = configData;

	server &Server = getServer(clientFd, configData);

	HttpRequest req = getReq(Server, clientFd);

	if (req.method == INVALID) {
		return ;
	}

	HttpResponse &res = Server.serverContex.responses[clientFd];

	normelaizePaths(req, Server);

	location *foundLocation = matchLocation(Server, req.path, req);

	if (foundLocation) {

		if (foundLocation->client_max_body_size != 0) {
			if (req.content_length >= foundLocation->client_max_body_size) {
				handle413(res);
				return;
			}
		} else if (Server.client_max_body_size != 0) {
			if (req.content_length >= Server.client_max_body_size) {
				handle413(res);
				return;
			}
		}

		if (!foundLocation->redirect.empty()) {
			bool isValidLocationName = false;
			std::string red = foundLocation->redirect;
			for (size_t i = 0; i < Server.locations.size(); i++) {
				if (red == Server.locations[i].name) {
					req.path = foundLocation->redirect + req.path.substr(foundLocation->name.size());
					foundLocation = &Server.locations[i];
					isValidLocationName = true;
					break;
				}
			}
			if (isValidLocationName == false) {
				handle404(res);
				return;
			}
		}

		Logger::debug("Matched location: %s", foundLocation->name.c_str());

		if (req.method == GET) {
			if (!foundLocation->get) {
				Logger::error("Method GET not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("GET in location %s called", foundLocation->name.c_str());
				routeRequestGET(req, res, Server, *foundLocation, clientFd);
			}
		} else if (req.method == POST) {
			if (!foundLocation->post) {
				Logger::error("Method POST not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("POST in location %s called", foundLocation->name.c_str());
				routeRequestPOST(req, res, Server, *foundLocation, clientFd);
			}
		} else if (req.method == DELETE) {
			if (!foundLocation->del) {
				Logger::error("Method DELETE not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("DELETE in location %s called", foundLocation->name.c_str());
				routeRequestDELETE(req, res, Server, *foundLocation);
			}
		} else {
			Logger::error("Method not allowed: %i", req.method);
			handle405(res);
		}
	} else {
		Logger::error("Unknown location or file for path: %s", req.path.c_str());
		handle404(res);
	}

	res.version = req.version;
	res.state = SENDING_HEADERS;

	if (req.method == HEAD) {
		res.body.clear();
	}

	addResponseEpoll(Server, clientFd, configData, res);

	res.startTime = getCurrentTime();
}

