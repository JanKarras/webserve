#include "../../include/webserv.hpp"

std::string stripTrailingSlash(const std::string &path) {
	if (path.length() > 1 && path[path.length() - 1] == '/')
		return path.substr(0, path.length() - 1);
	return path;
}

bool findInDirTree(const dir &current, const std::string &targetPath, SearchResult &result) {
	std::string normalizedTarget = stripTrailingSlash(targetPath);

	if (current.path == normalizedTarget) {
		result.found = true;
		result.isDir = true;
		result.foundDir = current;
		return true;
	}

	for (size_t i = 0; i < current.files.size(); ++i) {
		if (current.files[i].path == normalizedTarget) {
			result.found = true;
			result.isDir = false;
			result.foundFile = current.files[i];
			return true;
		}
	}

	for (size_t i = 0; i < current.dirs.size(); ++i) {
		if (findInDirTree(current.dirs[i], normalizedTarget, result)) {
			return true;
		}
	}

	return false;
}
