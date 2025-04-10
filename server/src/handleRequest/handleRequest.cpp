#include "../../include/webserv.hpp"

void handleRequest(int clientFd, ConfigData &configData) {
	size_t index = 0;
	std::map<int, ConfigData> data;
	data[configData.port] = configData;

	// Server anhand der Client-Anfrage identifizieren
	for (size_t i = 0; i < configData.servers.size(); i++) {
		server &server = configData.servers[i];
		ServerContext &serverContext = server.serverContex;
		if (serverContext.requests.find(clientFd) != serverContext.requests.end()) {
			index = i;
			Logger::info("Request was found for server: %s index = %zu", server.server_name.c_str(), index);
			break;
		}
	}
	server &Server = configData.servers[index];

	// Request und Response abrufen
	HttpRequest req = Server.serverContex.requests[clientFd];
	HttpResponse &res = Server.serverContex.responses[clientFd];

	printHttpRequest(req);

	// Beste passende Location finden (Längste-Prefix-Matching)
	location *bestMatch = NULL;
	size_t longestMatch = 0;
	std::string &path = req.path;

	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];

		// Prüfen, ob der Pfad mit der Location beginnt
		if (path.find(loc.name) == 0) {
			// Längere Matches haben Priorität
			if (loc.name.length() > longestMatch) {
				bestMatch = &loc;
				longestMatch = loc.name.length();
			}
		}
	}

	// Falls keine passende Location gefunden wurde, prüfen wir die Files in allen Locations
	if (!bestMatch) {
		for (size_t i = 0; i < Server.locations.size(); i++) {
			location &loc = Server.locations[i];

			for (size_t j = 0; j < loc.files.size(); j++) {
				std::string filePath = loc.files[j].path;

				// Falls das File unter dem `root`-Pfad gespeichert ist, korrigieren wir den relativen Pfad
				if (filePath.find(Server.root) == 0) {
					filePath = filePath.substr(Server.root.size());
				}

				// Wenn der Pfad genau mit einer Datei übereinstimmt, wählen wir diese Location
				if (path == filePath) {
					bestMatch = &loc;
					break;
				}
			}

			if (bestMatch) break;
		}
	}

	location *foundLocation = bestMatch;



	// Request basierend auf der gefundenen Location oder Datei verarbeiten
	if (foundLocation) {

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
				routeRequestGET(req, res, Server, *foundLocation);
			}
		} else if (req.method == POST) {
			if (!foundLocation->post) {
				Logger::error("Method POST not allowed for location %s", foundLocation->name.c_str());
				handle405(res);
			} else {
				Logger::debug("POST in location %s called", foundLocation->name.c_str());
				routeRequestPOST(req, res, Server, *foundLocation);
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
			Logger::error("Unknown method: %i", req.method);
			handle501(res);
		}
	} else {
		Logger::error("Unknown location or file for path: %s", req.path.c_str());
		handle404(res);
	}

	// Antwort vorbereiten
	res.version = req.version;
	res.state = SENDING_HEADERS;

	// Epoll für das Schreiben vorbereiten
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

