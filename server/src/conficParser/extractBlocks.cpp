#include "../../include/webserv.hpp"

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

std::vector<std::string> extractLocationBlocks(std::string &serverBlock) {
	std::vector<std::string> locationBlocks;
	std::string currentBlock;
	int braceCount = 0;
	bool insideLocation = false;

	std::istringstream fs(serverBlock);
	std::string line;

	while (std::getline(fs, line)) {
		if (line.find("location") != std::string::npos && braceCount == 0) {
			if (insideLocation) {
				locationBlocks.push_back(currentBlock);
				currentBlock.clear();
			}
			insideLocation = true;
			currentBlock = line;
			braceCount = 1;
		} else if (insideLocation) {
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
				locationBlocks.push_back(currentBlock);
				currentBlock.clear();
				insideLocation = false;
			}
		}
	}
	
	if (insideLocation && !currentBlock.empty()) {
		locationBlocks.push_back(currentBlock);
	}

	return locationBlocks;
}
