#include "../../include/webserv.hpp"

std::string trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\n\r");
	size_t last = str.find_last_not_of(" \t\n\r");
	if (first == std::string::npos || last == std::string::npos)
		return "";
	return str.substr(first, last - first + 1);
}

std::string removeQuotes(const std::string& str) {
	if (str.size() >= 2 && str[0] == '"' && str[str.size() - 1] == '"') {
		return str.substr(1, str.size() - 2);
	}
	return str;
}

std::map<std::string, std::string> parseSimpleJSON(const std::string& body) {
	std::map<std::string, std::string> jsonMap;

	std::string content = body;
	if (content.size() >= 2 && content[0] == '{' && content[content.size() - 1] == '}') {
		content = content.substr(1, content.size() - 2);
	}

	std::istringstream ss(content);
	std::string pair;

	while (std::getline(ss, pair, ',')) {
		size_t colonPos = pair.find(':');
		if (colonPos == std::string::npos) continue;

		std::string key = trim(pair.substr(0, colonPos));
		std::string value = trim(pair.substr(colonPos + 1));

		key = removeQuotes(key);
		value = removeQuotes(value);

		jsonMap[key] = value;
	}

	return jsonMap;
}
