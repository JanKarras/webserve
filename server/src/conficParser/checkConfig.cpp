#include "../../include/webserv.hpp"

bool isDirectoryAndReadable(const char* path) {
	struct stat pathStat;
	if (stat(path, &pathStat) == -1) {
		Logger::error("Directory check failed: Path : %s does not exist or cannot be accessed", path);
		return false;
	}

	if (!S_ISDIR(pathStat.st_mode)) {
		Logger::error("Directory check failed: Path : %s is not a directory", path);
		return false;
	}

	if (access(path, R_OK) == -1) {
		Logger::error("Directory check failed: No read permission for the directory with the path : %s", path);
		return false;
	}

	return true;
}

bool isFileAndReadable(const char* path) {
	struct stat pathStat;
	if (stat(path, &pathStat) == -1) {
		Logger::error("File check failed: Path : %s does not exist or cannot be accessed", path);
		return false;
	}

	if (!S_ISREG(pathStat.st_mode)) {
		Logger::error("File check failed: Path : %s is not a regular file", path);
		return false;
	}

	if (access(path, R_OK) == -1) {
		Logger::error("File check failed: No read permission for the file with the path : %s", path);
		return false;
	}

	return true;
}

bool isFileAndReadableAndExecuteable(const char* path) {
	struct stat pathStat;
	if (stat(path, &pathStat) == -1) {
		std::cerr << "File check failed: Path " << path << " does not exist or cannot be accessed.\n";
		return false;
	}

	if (!S_ISREG(pathStat.st_mode)) {
		std::cerr << "File check failed: Path " << path << " is not a regular file.\n";
		return false;
	}

	if (access(path, R_OK | X_OK) == -1) {
		Logger::error("File check failed: No read/execute permission for the file : %s", path);
		return false;
	}

	return true;
}

bool checkConfig(std::map<int, ConfigData> &data) {
	for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
		ConfigData &tmpData = it->second;
		if (tmpData.servers[0].server_name.empty()) {
			Logger::error("Standart server does not have a server name");
			return (false);
		}
		for (std::vector<server>::iterator itt = tmpData.servers.begin(); itt != tmpData.servers.end(); ++itt) {
			server &tmpServer = *itt;
			if(!isDirectoryAndReadable(tmpServer.root.c_str())) {
				return (false);
			}
			if (!isFileAndReadable((tmpServer.root + tmpServer.index).c_str())) {
				return (false);
			}
			for (std::vector<errorPage>::iterator ittt = tmpServer.errorpages.begin(); ittt != tmpServer.errorpages.end(); ++ittt) {
				errorPage &tmpErrorPage = *ittt;
				if (!isFileAndReadable((tmpServer.root + tmpErrorPage.path).c_str())) {
					return (false);
				}
			}

			for (std::vector<location>::iterator ittt = tmpServer.locations.begin(); ittt != tmpServer.locations.end(); ++ittt) {
				location &tmpLocation = *ittt;
				if (!tmpLocation.regularLocation) {
					if (tmpLocation.name.empty()) {
						Logger::error("Location with no name detected");
						return (false);
					}
				} else {
					if (tmpLocation.root.empty() || tmpLocation.index.empty()) {
						Logger::error("Regular locatoin with no root or index detected");
						return (false);
					}
				}
				if (!tmpLocation.root.empty()) {
					if (!isDirectoryAndReadable(tmpLocation.root.c_str())) {
						return (false);
					}
					if (!tmpLocation.index.empty()) {
						if (!isFileAndReadable((tmpLocation.root + "/" + tmpLocation.index).c_str())) {
							return (false);
						}
					}
				} else if (!tmpLocation.index.empty()) {
					if (!isFileAndReadable((tmpServer.root + tmpLocation.index).c_str())) {
						return (false);
					}
				}
				if (tmpLocation.cgi_paths.size() != 0) {
					for (std::vector<std::string>::iterator iteratorCgiPaths = tmpLocation.cgi_paths.begin(); iteratorCgiPaths != tmpLocation.cgi_paths.end(); ++iteratorCgiPaths) {
						std::string path = *iteratorCgiPaths;
						if (!isFileAndReadableAndExecuteable(path.c_str())) {
							return (false);
						}
					}
				}
				if (!tmpLocation.redirect.empty()) {
					std::string locationName = tmpLocation.redirect;
					bool flag = false;


					for (std::vector<location>::iterator iteratorLocation = tmpServer.locations.begin(); iteratorLocation != tmpServer.locations.end(); ++iteratorLocation) {
						location underLocation = *iteratorLocation;
						if (underLocation.name == locationName) {
							flag = true;
						}
					}
					if (flag == false) {
						Logger::error("No location for redirect found");
						return (false);
					}
				}
			}
		}

	}
	return true;
}
