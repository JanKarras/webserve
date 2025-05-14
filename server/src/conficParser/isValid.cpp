#include "../../include/webserv.hpp"

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
