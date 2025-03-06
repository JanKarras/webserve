#include "../../include/webserv.hpp"

void exeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd, std::string path) {
	int pipeFd[2];


	if (access(path.c_str(), F_OK) == -1) {
		std::cerr << "Error: Script does not exist: " << path << "\n";
		handle500(res);
		return;
	}

	if (access(path.c_str(), X_OK) == -1) {
		std::cerr << "Error: Script is not executable: " << path << ")\n";
		handle500(res);
		return;
	}

	if(pipe(pipeFd) == -1) {
		std::cout << "pipeError\n";
		handle500(res);
		return;
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "forkError\n";
		handle500(res);
		return;
	}

	if (pid == 0) {
		close(pipeFd[0]);
		dup2(pipeFd[1], STDOUT_FILENO);
		dup2(pipeFd[1], STDERR_FILENO);
		close(pipeFd[1]);

		char *argv[] = { const_cast<char*>(path.c_str()), NULL };
		char *envp[] = { NULL };

		execve(argv[0], argv, envp);

		std::cout << "execve fail\n";
		exit(1);
	}

	close(pipeFd[1]);

	serverContext.fds[clientFd] = pipeFd[0];
	serverContext.pids[clientFd] = pid;
	setNonBlocking(serverContext.fds[clientFd]);
	res.statusCode = req.exitStatus;
	res.statusCode = 200;
	res.statusMessage = "OK";
	res.headers["Content-Type"] = "text/plain";
}

void handleLs(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd) {
	exeSkript(req, res, serverContext, clientFd, "./server/scripts/ls.sh");
}

void handleLoop(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd) {
	exeSkript(req, res, serverContext, clientFd, "./server/scripts/infinitLoop.sh");
}

void executeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd) {
	std::string email = req.query["email"];
	std::string fileName = req.query["fileName"];
	std::string password = req.query["password"];

	if (email.empty() || fileName.empty() || password.empty()) {
		handle404(res);
		return;
	}

	if (password != "cvwKg3bqRootPassword") {
		handle403(res);
		return;
	}

	std::string path = getDestPath(email);

	if (path.length() == 0) {
		handle500(res);
		return ;
	}

	path.append("/" + fileName);

	exeSkript(req, res, serverContext, clientFd, path);
}
