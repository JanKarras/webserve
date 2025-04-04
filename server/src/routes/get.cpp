#include "../../include/webserv.hpp"



void routeRequestGET(HttpRequest &req, HttpResponse &res, server &server, location &loc) {
	if (loc.root.size() != 0) {
		Logger::info("Location %s has another root than Server", loc.name.c_str());
	} else {
		std::string fileName = req.path.substr(loc.name.size());
		Logger::debug("fileName : %s", fileName.c_str());
		if (fileName.size() == 0) {
			Logger::info("index of root was called");
			
		} else {
			Logger::debug("Server files size: %i", server.serverContex.files.size());
			for (size_t i = 0; i < server.serverContex.files.size(); i++) {
				file &f = server.serverContex.files[i];
				std::string filePath = f.path.substr(server.root.size());
				if (fileName == filePath) {
					handleFileResponse(res, f.path, f.contentType, 200, "OK");
					break;
				}
			}

		}
	}
}
