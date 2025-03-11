#include "../../include/webserv.hpp"

int main(int argc, char **argv) {

	std::map<int, ConfigData> data;


	if (argc > 2) {
		Logger::error("Bad number of arguments. Provided: %i Allowed: 0 or 1", argc - 1);
		return (1);
	} else if (argc == 2) {
		if (parseConfic(std::string(argv[1]), data)) {
			return (1);
		} else {
			if(!initSignal()) {
				return (1);
			}
			startServer(data);
		}
	} else {
		if (parseConfic("std_confic", data)) {
			return (1);
		} else {
			if(!initSignal()) {
				return (1);
			}
			startServer(data);
		}
	}
	return (0);
}
