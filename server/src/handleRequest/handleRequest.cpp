#include "../../include/webserv.hpp"

bool matchPathInDirTree(const dir &currentDir, const std::string &reqPath) {
	// Prüfe alle Files im aktuellen Verzeichnis
	for (size_t i = 0; i < currentDir.files.size(); ++i) {
		if (currentDir.files[i].path == reqPath) {
			return true;
		}
	}

	// Prüfe den aktuellen Dir selbst
	if (currentDir.path == reqPath) {
		return true;
	}

	// Rekursiv in allen Unterverzeichnissen prüfen
	for (size_t j = 0; j < currentDir.dirs.size(); ++j) {
		if (matchPathInDirTree(currentDir.dirs[j], reqPath)) {
			return true;
		}
	}
	return false;
}

location* matchLocation(server &Server, const std::string &path, HttpRequest &req) {
	location *regexMatch = NULL;
	location *prefixMatch = NULL;
	size_t longestPrefix = 0;

	// 1. Extension/Regex-Match mit Priorität (aber nur wenn Methode erlaubt ist!)
	Logger::debug("Checking for Regular Location (extension-based)");
	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];
		if (loc.regularLocation) {
			Logger::debug("Location ext: %s, path: %s", loc.ext.c_str(), path.c_str());
			if (path.size() >= loc.ext.size() &&
				path.compare(path.size() - loc.ext.size(), loc.ext.size(), loc.ext) == 0) {

				// 💡 Prüfe, ob Methode erlaubt ist
				bool methodAllowed =
					(req.method == GET && loc.get) ||
					(req.method == POST && loc.post) ||
					(req.method == DELETE && loc.del);

				if (methodAllowed) {
					Logger::debug("Regex match found (method allowed): %s", loc.name.c_str());
					return &loc; // sofort zurückgeben
				} else {
					Logger::debug("Regex match found, but method not allowed => ignoring");
				}
			}
		}
	}

	// 2. Längstes Prefix-Match suchen
	Logger::debug("Checking for Path Prefix Match");
	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];
		if (!loc.regularLocation && path.find(loc.name) == 0) {
			if (loc.name.length() > longestPrefix) {
				prefixMatch = &loc;
				longestPrefix = loc.name.length();
			}
		}
	}

	if (prefixMatch) {
		Logger::debug("Prefix match found: %s", prefixMatch->name.c_str());
		return prefixMatch;
	}

	// 3. DirTree-Fallback
	Logger::debug("No Prefix or Regex Match. Checking Dir Tree...");
	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];
		if (matchPathInDirTree(loc.tree, path)) {
			Logger::debug("DirTree match found: %s", loc.name.c_str());
			return &loc;
		}
	}

	return NULL;
}



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
	std::map<int, HttpRequest *>::iterator it = Server.serverContex.requests.find(clientFd);
	HttpRequest req;
	if (it != Server.serverContex.requests.end()) {
		req =  *it->second;
	} else {
		std::cout << "error" << std::endl;
		return;
	}
	HttpResponse &res = Server.serverContex.responses[clientFd];

	// Normalisiere Pfad für Matching: Wenn eine Location mit /xyz/ existiert und Pfad = /xyz, hänge / an
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

	location *foundLocation = matchLocation(Server, req.path, req);


	// Request basierend auf der gefundenen Location oder Datei verarbeiten
	if (foundLocation) {

		//Logger::debug("content length: %i location body size: %i server body size %i", req.content_length, foundLocation->client_max_body_size, Server.client_max_body_size);

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

	// Antwort vorbereiten
	res.version = req.version;
	res.state = SENDING_HEADERS;

	// Epoll für das Schreiben vorbereiten

	if (req.method == HEAD) {
		res.body.clear();
	}

	if (Server.serverContex.cgifds.find(clientFd) != Server.serverContex.cgifds.end()) {
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = Server.serverContex.cgifds[clientFd];
		Logger::debug("Fd eppol out add %i", Server.serverContex.cgifds[clientFd]);
		if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, Server.serverContex.cgifds[clientFd], &event) == -1) {
			std::cerr << "Failed to modify epoll event for EPOLLOUT." << std::endl;
			close(clientFd);
			Server.serverContex.requests.erase(clientFd);
			Server.serverContex.responses.erase(clientFd);
			return;
		}
	}

	if (Server.serverContex.fds.find(clientFd) != Server.serverContex.fds.end()) {
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLHUP;
		event.data.fd = Server.serverContex.fds[clientFd];
		if (epoll_ctl(configData.epollFd, EPOLL_CTL_ADD, Server.serverContex.fds[clientFd], &event) == -1) {
			std::cerr << "Failed to modify epoll event for EPOLLIN." << std::endl;
			close(clientFd);
			Server.serverContex.requests.erase(clientFd);
			Server.serverContex.responses.erase(clientFd);
			return;
		}
		res.state = NOT_STARTED;
	}
	struct epoll_event event;
	event.events = EPOLLOUT;
	event.data.fd = clientFd;

	if (epoll_ctl(configData.epollFd, EPOLL_CTL_MOD, clientFd, &event) == -1) {
		std::cerr << "Failed to modify epoll event for EPOLLOUT respons." << std::endl;
		close(clientFd);
		Server.serverContex.requests.erase(clientFd);
		Server.serverContex.responses.erase(clientFd);
		return;
	}

	res.startTime = getCurrentTime();
}

