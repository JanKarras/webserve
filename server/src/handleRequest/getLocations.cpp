#include "../../include/webserv.hpp"

bool matchPathInDirTree(const dir &currentDir, const std::string &reqPath) {
	for (size_t i = 0; i < currentDir.files.size(); ++i) {
		if (currentDir.files[i].path == reqPath) {
			return true;
		}
	}

	if (currentDir.path == reqPath) {
		return true;
	}

	for (size_t j = 0; j < currentDir.dirs.size(); ++j) {
		if (matchPathInDirTree(currentDir.dirs[j], reqPath)) {
			return true;
		}
	}
	return false;
}

location* matchLocation(server &Server, const std::string &path, HttpRequest &req) {
	location *prefixMatch = NULL;
	size_t longestPrefix = 0;

	// Logger::debug("Checking for Regular Location (extension-based)");
	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];
		if (loc.regularLocation) {
			// Logger::debug("Location ext: %s, path: %s", loc.ext.c_str(), path.c_str());
			if (path.size() >= loc.ext.size() &&
				path.compare(path.size() - loc.ext.size(), loc.ext.size(), loc.ext) == 0) {

				bool methodAllowed =
					(req.method == GET && loc.get) ||
					(req.method == POST && loc.post) ||
					(req.method == DELETE && loc.del);

				if (methodAllowed) {
					// Logger::debug("Regex match found (method allowed): %s", loc.name.c_str());
					return &loc;
				} else {
					// Logger::debug("Regex match found, but method not allowed => ignoring");
				}
			}
		}
	}

	// Logger::debug("Checking for Path Prefix Match");
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
		// Logger::debug("Prefix match found: %s", prefixMatch->name.c_str());
		return prefixMatch;
	}

	// Logger::debug("No Prefix or Regex Match. Checking Dir Tree...");
	for (size_t i = 0; i < Server.locations.size(); i++) {
		location &loc = Server.locations[i];
		if (matchPathInDirTree(loc.tree, path)) {
			// Logger::debug("DirTree match found: %s", loc.name.c_str());
			return &loc;
		}
	}

	return NULL;
}
