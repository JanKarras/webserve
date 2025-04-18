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

	std::string method;

	if (req.method == GET) {
		method = "REQUEST_METHOD=GET";
	} else if (req.method == POST) {
		method = "REQUEST_METHOD=POST";
	} else {
		method = "REQUEST_METHOD=DELETE";
	}

	if (pid == 0) {
		close(pipeFd[0]);
		dup2(pipeFd[1], STDOUT_FILENO);
		dup2(pipeFd[1], STDERR_FILENO);
		close(pipeFd[1]);

		char *argv[] = { const_cast<char*>(path.c_str()), NULL };
		char *envp[] = {	const_cast<char*>("REQUEST_METHOD=POST"),
							const_cast<char*>("SERVER_PROTOCOL=HTTP/1.1"),
							NULL };

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

void executeSkript(HttpRequest &req, HttpResponse &res, server &server, int clientFd, file f) {
	Logger::debug("executer script f.path: %s", f.path.c_str());
	exeSkript(req, res, server.serverContex, clientFd, f.path);
}
