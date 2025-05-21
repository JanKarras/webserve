#include "../../include/webserv.hpp"

bool setsetExecutable(std::string &filePath) {
	pid_t pid = fork();

	if (pid == -1) {
		Logger::error("Fork failed");
		return false;
	}

	if (pid == 0) {
		// Kindprozess führt chmod aus
		char *argv[] = { (char *)"/bin/chmod", (char *)"755", (char *)filePath.c_str(), NULL };
		execve(argv[0], argv, NULL);

		// Falls execve fehlschlägt, beenden wir den Kindprozess sofort
		Logger::error("execve failed for chmod");
		_exit(1);
	}

	// Elternprozess wartet auf Kindprozess
	int status;
	if (waitpid(pid, &status, 0) == -1) {
		Logger::error("waitpid failed");
		return false;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		//Logger::info("File set to executable: %s", filePath.c_str());
		return true;
	} else {
		Logger::error("chmod failed with exit code %d", WEXITSTATUS(status));
		return false;
	}
}

