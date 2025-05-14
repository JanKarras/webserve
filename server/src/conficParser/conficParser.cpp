#include "../../include/webserv.hpp"



bool checkPath(std::string path) {
	if (access(path.c_str(), F_OK) != 0) {
		Logger::error("File does not exists\n");
		return (false);
	}

	if (access(path.c_str(), R_OK) != 0) {
		Logger::error("File is not readable\n");
		return (false);
	}

	return (true);
}

std::string readFile(std::string path) {

	std::ifstream file(path.c_str());

	if (!file) {
		Logger::error("Reading file\n");
		return ("");
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	file.close();

	return (buffer.str());
}

std::vector<std::string> extractLocationBlocks(std::string &serverBlock) {
	std::vector<std::string> locationBlocks;
	std::string currentBlock;
	int braceCount = 0;
	bool insideLocation = false;

	std::istringstream fs(serverBlock);
	std::string line;

	while (std::getline(fs, line)) {
		// Überprüfen, ob wir mit einem Location-Block beginnen
		if (line.find("location") != std::string::npos && braceCount == 0) {
			if (insideLocation) {
				// Falls wir bereits in einem Location-Block sind, müssen wir sicherstellen, dass wir den vorherigen Block beenden.
				locationBlocks.push_back(currentBlock);
				currentBlock.clear();
			}
			insideLocation = true;
			currentBlock = line;  // Start des Location-Blocks
			braceCount = 1;  // Ersten offenen Brace zählen
		} else if (insideLocation) {
			currentBlock += '\n' + line;  // Zeile zum aktuellen Location-Block hinzufügen

			// Zählen der öffnenden und schließenden Klammern
			size_t openingBraces = 0;
			size_t closingBraces = 0;

			for (size_t i = 0; i < line.length(); ++i) {
				if (line[i] == '{') {
					openingBraces++;
				} else if (line[i] == '}') {
					closingBraces++;
				}
			}

			braceCount += openingBraces;
			braceCount -= closingBraces;

			// Wenn alle Klammern geschlossen sind, Location-Block abschließen
			if (braceCount == 0) {
				locationBlocks.push_back(currentBlock);
				currentBlock.clear();
				insideLocation = false;
			}
		}
	}

	// Falls der letzte Location-Block nicht hinzugefügt wurde (bei mehreren Locations im Block)
	if (insideLocation && !currentBlock.empty()) {
		locationBlocks.push_back(currentBlock);
	}

	return locationBlocks;
}


std::vector<std::string> extractServerBlocks(std::string &file) {
	std::vector<std::string> serverBlocks;
	std::string currentBlock;
	int braceCount = 0;

	std::istringstream fs(file);
	std::string line;

	while (std::getline(fs, line)) {
		if (line.find("server {") != std::string::npos && braceCount == 0) {
			braceCount++;
			currentBlock = line;
		} else if (braceCount > 0) {
			currentBlock += '\n' + line;

			size_t openingBraces = 0;
			size_t closingBraces = 0;

			for (size_t i = 0; i < line.length(); ++i) {
				if (line[i] == '{') {
					openingBraces++;
				} else if (line[i] == '}') {
					closingBraces++;
				}
			}

			braceCount += openingBraces;
			braceCount -= closingBraces;

			if (braceCount == 0) {
				serverBlocks.push_back(currentBlock);
				currentBlock.clear();
			}
		}
	}
	return serverBlocks;
}


bool isValidConfigLine(const std::string &line, const std::string &expectedKeyword, server &Server) {
	std::istringstream ss(line);
	std::string keyword;
	std::string value;

	ss >> keyword;
	if (keyword != expectedKeyword) {
		Logger::error("%s line does not start with '%s'", keyword.c_str(), expectedKeyword.c_str());
		return true;
	}

	std::getline(ss >> std::ws, value, ';');
	if (value.empty()) {
		Logger::error("%s line has no value", expectedKeyword.c_str());
		return true;
	}

	if (line[line.size() - 1] != ';') {
		Logger::error("%s line doesn't end with ';'", expectedKeyword.c_str());
		return true;
	}

	std::string remaining;
	if (ss >> remaining) {
		Logger::error("%s line contains extra characters after ';'", expectedKeyword.c_str());
		return true;
	}
	if (keyword == "server_name") {
		Server.server_name = value;
	} else if (keyword == "root") {
		Server.root = value;
	} else if (keyword == "host") {
		Server.host = value;
	} else if (keyword == "index") {
		Server.index = value;
	}
	return false;
}

bool isValidIntConfigLine(const std::string &line, const std::string &expectedKeyword, int minValue, int maxValue, server &Server) {
	std::istringstream ss(line);
	std::string keyword;
	int value;
	char semicolon;

	ss >> keyword >> value >> semicolon;

	if (keyword != expectedKeyword) {
		Logger::error("%s line does not start with '%s'", keyword.c_str(), expectedKeyword.c_str());
		return true;
	}

	if (value < minValue || value > maxValue) {
		Logger::error("%s value out of range (%d - %d). Given: %d", expectedKeyword.c_str(), minValue, maxValue, value);
		return true;
	}

	if (semicolon != ';') {
		Logger::error("%s line doesn't end with ';'", expectedKeyword.c_str());
		return true;
	}

	std::string remaining;
	if (ss >> remaining) {
		Logger::error("%s line contains extra characters after ';'", expectedKeyword.c_str());
		return true;
	}

	if (keyword == "listen") {
		Server.port = value;
	} else if (keyword == "client_max_body_size") {
		Server.client_max_body_size = value;
	}

	return false;
}

bool isValidErrorPageLine(const std::string &line, server &Server) {
	std::istringstream ss(line);
	std::string keyword;
	size_t errorCode;
	std::string path;

	ss >> keyword >> errorCode >> path;

	if (keyword != "error_page") {
		Logger::error("Error page line does not start with 'error_page'");
		return true;
	}

	if (errorCode < 400 || errorCode > 599) {
		Logger::error("Invalid error code in error_page line: %zu", errorCode);
		return true;
	}

	if (path.empty() || path[0] == ';') {
		Logger::error("Missing or invalid path in error_page line");
		return true;
	}

	path = path.substr(0, path.size() - 1);

	std::string remaining;
	if (ss >> remaining) {
		Logger::error("Error page line contains extra characters after ';'");
		return true;
	}



	errorPage errorPage;
	errorPage.errorCode = errorCode;
	errorPage.path = path;

	Server.errorpages.push_back(errorPage);


	return false;
}

bool isValidLocation(const std::string &line, location &Location) {
	Location.get = false;
	Location.post = false;
	Location.del = false;
	Location.client_max_body_size = 0;
	Location.regularLocation = false;
	std::istringstream ss(line);
	std::string keyword;
	std::string value;
	std::string subline;

	while (getline(ss, subline)) {
		if (!subline.empty() && subline[subline.size() - 1] == ';') {
			subline = subline.substr(0, subline.size() - 1);
		}
		std::istringstream iss(subline);
		iss >> keyword;


		if (keyword == "allow_methods") {
			while (iss >> value) {
				if (value == "GET") {
					Location.get = true;
				} else if (value == "POST"){
					Location.post = true;
				}
				else if (value == "DELETE") {
					Location.del  = true;
				} else {
					Logger::error("Unknown method '%s' in location block", value.c_str());
					return true;
				}
			}
		} else if (keyword == "index") {
			iss >> value;
			Location.index = value;
		} else if (keyword == "client_max_body_size") {
			std::string valStr;
			iss >> valStr;

			std::istringstream issVal(valStr);
			int val;
			char c;

			if (!(issVal >> val) || (issVal >> c)) {
				Logger::error("Invalid client_max_body_size: '%s'", valStr.c_str());
				return true;
			}
			if (val < 0) {
				Logger::error("client_max_body_size must not be negative: '%s'", valStr.c_str());
				return true;
			}
	Location.client_max_body_size = val;
		} else if (keyword == "root") {
			iss >> value;
			Location.root = value;
		} else if (keyword == "cgi_path") {
			while (iss >> value) {
				Location.cgi_paths.push_back(value);
			}
		} else if (keyword == "cgi_ext") {
			while (iss >> value) {
				Location.cgi_ext.push_back(value);
			}
		} else if (keyword == "return") {
			iss >> value;
			Location.redirect = value;
		} else if (keyword == "location") {
			std::string tmp;
			iss >> tmp;
			if (tmp == "~") {
				Location.regularLocation = true;
				std::string regexPattern;
				iss >> regexPattern;
				Location.pattern = regexPattern;
				if (regexPattern.size() >= 4 && regexPattern.substr(0, 2) == "\\." && regexPattern[regexPattern.size() - 1] == '$') {
					Location.ext = "." + regexPattern.substr(2, regexPattern.size() - 3);
				} else {
					Logger::error("Unsupported regex location pattern: '%s'", regexPattern.c_str());
					return true;
				}
			} else {
				Location.name = tmp;
				Location.regularLocation = false;
			}
		} else if (keyword == "}") {
			continue;
		} else {
			Logger::error("Unknown keyword '%s' in location block", keyword.c_str());
			return true;
		}
	}

	return false;
}



bool parseServer(std::map<int, ConfigData> &data, std::string serverBlock) {
	std::string line;
	std::istringstream fs(serverBlock);
	std::vector<std::string> lines;
	server Server;

	while (std::getline(fs, line)) {
		if (line.empty()) {
			Logger::error("Empty line in config file");
			return (false);
		}
		lines.push_back(line);
	}

	if (isValidIntConfigLine(lines[1], "listen", 1, 65535, Server)) {
		Logger::error("Invalid 'listen' line");
		return (false);
	}

	if (isValidConfigLine(lines[2], "server_name", Server)) {
		Logger::warn("Invalid 'server_name' line");
	}

	if (isValidConfigLine(lines[3], "host", Server)) {
		Logger::warn("Invalid 'host' line");
	}

	if (isValidConfigLine(lines[4], "root", Server)) {
		Logger::error("Invalid 'root' line");
		return false;
	}

	if (isValidIntConfigLine(lines[5], "client_max_body_size", 0, INT32_MAX, Server)) {
		Logger::error("Invalid 'client_max_body_size' line");
		return (false);
	}

	if (isValidConfigLine(lines[6], "index", Server)) {
		Logger::error("Invalid 'index' line");
		return (false);
	}

	for (size_t i = 7; lines[i].find("error_page") != std::string::npos; i++) {
		if (isValidErrorPageLine(lines[i], Server)) {
			Logger::error("Invalid 'error_page'");
			return (false);
		}
	}

	std::vector<std::string> locationBlocks = extractLocationBlocks(serverBlock);

	for (size_t i = 0; i < locationBlocks.size(); i++) {
		location Location;
		if(isValidLocation(locationBlocks[i], Location)) {
			Logger::error("Invalid 'locationblock'");
			return (false);
		}
		Server.locations.push_back(Location);
	}

	if (data.find(Server.port) != data.end()) {
		data[Server.port].servers.push_back(Server);
	} else {
		data[Server.port] = ConfigData();
		data[Server.port].servers.push_back(Server);
		data[Server.port].port = Server.port;
	}

	return true;
}

bool validateLines(const std::string &filetxt) {
	std::istringstream stream(filetxt);
	std::string line;

	while (std::getline(stream, line)) {
		while (!line.empty() && (line[line.length() - 1] == ' ' || line[line.length() - 1] == '\t')) {
			line.erase(line.length() - 1);
		}

		char lastChar = line[line.length() - 1];
		if (lastChar != '{' && lastChar != '}' && lastChar != ';') {
			return false; // Eine Zeile endet nicht mit {, } oder ;
		}
	}
	return true;
}

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

bool parseConfig(std::string path, std::map<int, ConfigData> &data) {
	if (!checkPath(path)) {
		return (true);
	}

	std::string file = readFile(path);

	if (file.empty()) {
		return (true);
	}

	if(!validateLines(file)) {
		Logger::error("Malformed line found kek");
		return (true);
	}

	std::vector<std::string> serverBlocks = extractServerBlocks(file);

	if (serverBlocks.empty()) {
		return (true);
	}

	for (size_t i = 0; i < serverBlocks.size(); i++) {
		if (!parseServer(data, serverBlocks[i])) {
			Logger::error("Parsing server %i", i);
			return (true);
		}
	}

	if (!checkConfig(data)) {
		return true;
	}

	return (false);
}
