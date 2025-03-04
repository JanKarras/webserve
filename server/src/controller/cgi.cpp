#include "../../include/webserv.hpp"

void exeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd, std::string path) {
	int pipeFd[2];

	std::cout << path << std::endl;

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

	if (email.empty() || fileName.empty()) {
		handle404(res);
		return;
	}

	std::string path = getDestPath(email);

	std::cout << path << std::endl;

	if (path.length() == 0) {
		handle500(res);
		return ;
	}

	path.append("/" + fileName);

	exeSkript(req, res, serverContext, clientFd, path);
}
