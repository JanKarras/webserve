#include "../../include/webserv.hpp"

std::string getDestPath(std::string email) {
	std::string mainPath = "./server/files/";

	std::ifstream file("server/src/db/loginData.csv");
	if (!file) {
		std::cerr << "File not found\n";
		return ("");
	}

	std::string line;
	while (getline(file, line)) {
		std::stringstream ss(line);
		std::string emailDb, passwordDb, idStr;

		if (getline(ss, emailDb, '|') && getline(ss, passwordDb, '|') && getline(ss, idStr, '|')) {
			if (emailDb == email) {
				mainPath.append(idStr);
				return mainPath;
			}

		} else {
			std::cerr << "Error parsing line: " << line << "\n";
			file.close();
			return ("");
		}
	}
	file.close();
	return ("");
}
