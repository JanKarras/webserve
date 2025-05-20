#include "../../include/webserv.hpp"


std::string getFileContent(std::string filePath) {

	std::ifstream file(filePath.c_str());
	std::string body;

	if (file) {
		std::string line;
		while (getline(file, line)) {
			body+= line + "\n";
		}
		file.close();
	} else {
		Logger::error("File in getFileContent for path %s not found", filePath.c_str());
		return ("");
	}
	return body;
}

