#include "../../include/webserv.hpp"

void routeRequestGET(HttpRequest &req, HttpResponse &res, server &server, location &loc) {
	if (loc.root.size() != 0) {
		Logger::info("Location %s has another root than Server", loc.name.c_str());
		std::string fileName = req.path.substr(loc.name.size());
		Logger::debug("fileName : %s", fileName.c_str());
		if (fileName.size() == 0 || fileName == "/") {
			Logger::info("index of root was called");
			for (size_t i = 0; i < loc.files.size(); i++) {
				file &f = loc.files[i];
				std::string filePath = f.path.substr(loc.root.size() + 1);
				Logger::info("index: %s -- filePath: %s -- f.path: %s", loc.index.c_str(), filePath.c_str(), f.path.c_str());
				if (loc.index.size() == 0) {
					if (loc.index == filePath) {
						handleFileResponse(res, f.path, f.contentType, 200, "OK");
						return;
					}
				} else {
					if (loc.index == filePath) {
						handleFileResponse(res, f.path, f.contentType, 200, "OK");
						return;
					}
				}
			}
			handle404(res);
		} else {
			Logger::debug("Server files size: %i", loc.files.size());
			for (size_t i = 0; i < loc.files.size(); i++) {
				file &f =loc.files[i];
				std::string filePath = f.path.substr(loc.root.size());
				if (fileName == filePath) {
					handleFileResponse(res, f.path, f.contentType, 200, "OK");
					return;
				}
			}
			handle404(res);
		}
	} else {
		std::string fileName = req.path.substr(loc.name.size());
		Logger::debug("fileName : %s", fileName.c_str());
		if (fileName.size() == 0) {
			Logger::info("index of root was called");
			for (size_t i = 0; i < server.serverContex.files.size(); i++) {
				file &f = server.serverContex.files[i];
				std::string filePath = f.path.substr(server.root.size());
				Logger::info("index: %s -- filePath: %s", loc.index.c_str(), filePath.c_str());
				if (loc.index.size() == 0) {
					if (server.index == filePath) {
						handleFileResponse(res, f.path, f.contentType, 200, "OK");
						return;
					}
				} else {
					if (loc.index == filePath) {
						handleFileResponse(res, f.path, f.contentType, 200, "OK");
						return;
					}
				}
			}
			handle404(res);
		} else {
			Logger::debug("Server files size: %i", server.serverContex.files.size());
			for (size_t i = 0; i < server.serverContex.files.size(); i++) {
				file &f = server.serverContex.files[i];
				std::string filePath = f.path.substr(server.root.size());
				if (fileName == filePath) {
					handleFileResponse(res, f.path, f.contentType, 200, "OK");
					return;
				}
			}
			handle404(res);
		}
	}
}
