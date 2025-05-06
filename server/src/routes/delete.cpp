#include "../../include/webserv.hpp"

#include <cstdio>  // Für remove()
#include <cerrno>  // Für errno
#include <cstring> // Für strerror()
#include <unistd.h> // Für access()

void routeRequestDELETE(HttpRequest &req, HttpResponse &res, server &server, location &loc) {
	(void)req;
	(void)res;
	(void)server;
	(void)loc;
	// std::string fileName = req.path.substr(loc.name.size());
	// Logger::info("fileName: %s", fileName.c_str());

	// if (loc.root.size() != 0) {
	// 	Logger::info("Location file to delete: %s", req.path.c_str());
	// 	bool fileFound = false;
	// 	std::string fullPath;

	// 	std::vector<file>::iterator it;
	// 	for (it = loc.files.begin(); it != loc.files.end(); ++it) {
	// 		Logger::info("location file path: %s", it->path.c_str());

	// 		fullPath = loc.root + fileName;
	// 		if (it->path == fullPath) {
	// 			fileFound = true;
	// 			Logger::info("File found: %s", it->path.c_str());
	// 			break;
	// 		}
	// 	}

	// 	if (!fileFound) {
	// 		Logger::error("File not found: %s", fileName.c_str());
	// 		handle404(res);
	// 		return;
	// 	}

	// 	if (access(fullPath.c_str(), F_OK) != 0) {
	// 		Logger::error("File does not exist: %s", fullPath.c_str());
	// 		handle404(res);
	// 		return;
	// 	}

	// 	if (access(fullPath.c_str(), W_OK) != 0) {
	// 		Logger::error("No permission to delete file: %s", fullPath.c_str());
	// 		handle403(res);
	// 		return;
	// 	}

	// 	if (remove(fullPath.c_str()) == 0) {
	// 		Logger::info("File deleted successfully: %s", fullPath.c_str());
	// 	} else {
	// 		Logger::error("Failed to delete file: %s", strerror(errno));
	// 		handle500(res);
	// 	}
	// } else {
	// 	Logger::info("Server file to delete %s", req.path.c_str());
	// }
}


