#include "../../include/webserv.hpp"











bool parseConfig(std::string path, std::map<int, ConfigData> &data) {
	if (!checkPath(path)) {
		return (true);
	}

	std::string file = readFile(path);

	if (file.empty()) {
		return (true);
	}

	if(!validateLines(file)) {
		Logger::error("Malformed line found");
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
