#include "../../include/webserv.hpp"

void printServer(server &Server) {
	Logger::debug("Port: %i", Server.port);
	Logger::debug("client_max_body_size: %i", Server.client_max_body_size);
	Logger::debug("server_name: %s", Server.server_name.c_str());
	Logger::debug("host: %s", Server.host.c_str());
	Logger::debug("root: %s", Server.root.c_str());
	Logger::debug("index: %s", Server.index.c_str());
	Logger::debug("%s", Server.errorpages[0].path.c_str());
	for (size_t i = 0; i < Server.errorpages.size(); i++) {
		Logger::debug("Error Page %i: errorCode: %i, path: %s", i, Server.errorpages[i].errorCode, Server.errorpages[i].path.c_str());
	}
	for (size_t i = 0; i < Server.locations.size(); i++) {
		const location& loc = Server.locations[i];  // Referenz auf das aktuelle location-Objekt

		// Ausgabe der verschiedenen Eigenschaften der location
		Logger::debug("Location %d:", i);

		if (loc.get) {
			Logger::debug("  Get is true");
		}

		if (loc.post) {
			Logger::debug("  post is true");
		}

		if (loc.del) {
			Logger::debug("  del is true");
		}

		Logger::debug("  Name: %s", loc.name.c_str());
		Logger::debug("  Root: %s", loc.root.c_str());
		Logger::debug("  Index: %s", loc.index.c_str());

		// Ausgabe der CGI-Pfade
		Logger::debug("  CGI Paths: ");
		for (size_t j = 0; j < loc.cgi_paths.size(); j++) {
			Logger::debug("    - %s", loc.cgi_paths[j].c_str());
		}

		// Ausgabe der CGI-Erweiterungen
		Logger::debug("  CGI Extensions: ");
		for (size_t k = 0; k < loc.cgi_ext.size(); k++) {
			Logger::debug("    - %s", loc.cgi_ext[k].c_str());
		}

		// Ausgabe des Redirects (falls vorhanden)
		if (!loc.redirect.empty()) {
			Logger::debug("  Redirect: %s", loc.redirect.c_str());
		} else {
			Logger::debug("  Redirect: None");
		}
	}
}

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
				}
			}
		} else if (keyword == "index") {
			iss >> value;
			Location.index = value;
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
			iss >> value;
			Location.name = value;
		} else if (keyword == "}") {
			continue;
		} else {
			Logger::error("Unknown keyword '%s' in location block", keyword.c_str());
			return true;
		}

		// if (line[line.size() - 1] != ';' && keyword != "location") {
		// 	Logger::error("Line doesn't end with ';' in location block");
		// 	return true;
		// }
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

	if (isValidIntConfigLine(lines[5], "client_max_body_size", 1, 500000000, Server)) {
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


void printAll(std::map<int, ConfigData> &data) {
	for (std::map<int, ConfigData>::iterator it = data.begin(); it != data.end(); ++it) {
		std::cout << "-------------------------------------------------------------\n";
		std::cout << "Port:             " << it->second.port << std::endl;
		std::cout << "Keyvalue:         " << it->first << std::endl;
		std::cout << "Server number:    " << it->second.servers.size() << std::endl;
		int i = 0;
		for (std::vector<server>::iterator itt = it->second.servers.begin(); itt != it->second.servers.end(); ++itt) {
			std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~" << "Server: " << i << "~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
			printServer(*itt);
			std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
			i++;
		}
		std::cout << "-------------------------------------------------------------\n";
	}
}


bool parseConfic(std::string path, std::map<int, ConfigData> &data) {
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

	//printAll(data);

	return (false);
}
