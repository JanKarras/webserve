#include "../../include/webserv.hpp"

void redirectOutfile(std::string content, std::string filePath, size_t length, size_t bytesRead) {
	std::ofstream outFile(filePath.c_str(), std::ios::app);
	if (!outFile) {
		std::cerr << "Error opening file for writing.\n";
		return ;
	}

	if (bytesRead <= length) {
		outFile.write(content.c_str(), bytesRead);
	} else {
		outFile.write(content.c_str(), length);
		outFile.write("\n", 1);
	}

	if (outFile.fail()) {
		std::cerr << "Error writing to file.\n";
		return ;
	}

	outFile.close();
}
