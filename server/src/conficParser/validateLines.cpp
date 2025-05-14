#include "../../include/webserv.hpp"

bool validateLines(const std::string &filetxt) {
	std::istringstream stream(filetxt);
	std::string line;

	while (std::getline(stream, line)) {
		while (!line.empty() && (line[line.length() - 1] == ' ' || line[line.length() - 1] == '\t')) {
			line.erase(line.length() - 1);
		}

		char lastChar = line[line.length() - 1];
		if (lastChar != '{' && lastChar != '}' && lastChar != ';') {
			return false;
		}
	}
	return true;
}
