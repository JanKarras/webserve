#include "../../include/webserv.hpp"



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
