#include "../../include/webserv.hpp"

bool setsetExecutable(std::string &filePath) {
	pid_t pid = fork();

	if (pid == -1) {
		std::cout << "Error: fork failed\n";
		return false;
	}

	if (pid == 0) {
		char *argv[] = { (char *)"/bin/chmod", (char *)"755", (char *)filePath.c_str(), NULL };
		execve(argv[0], argv, NULL);

		std::cerr << "Error: execve failed\n";
		_exit(1);
	}

	int status;

	if (waitpid(pid, &status, 0) == -1) {
		std::cerr << "Error: waitpid failed\n";
		return false;
	}

	if (status == 0) {
		return true;
	} else {
		std::cerr << "Error: chmod failed with status code " << status << "\n";
		return false;
	}
}
