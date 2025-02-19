#include "../../include/webserv.hpp"


std::string getFileContent(std::string filePath) {
	std::string publicPath = "../../../";
	publicPath.append(filePath);
	std::ifstream file(publicPath.c_str());

	std::string body;

	if (file) {
		std::string line;
		while (getline(file, line)) {
			body+= line + "\n";
		}
		file.close();
	} else {
		std::cerr << "File not found\n";
	}
	return body;
}
