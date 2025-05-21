#include "../../include/webserv.hpp"

std::string httpMethodToString(HttpMethod method) {
	if (method == GET) return "GET";
	if (method == POST) return "POST";
	if (method == DELETE) return "DELETE";
	return "UNKNOWN";
}

std::vector<std::string> getEnvp(HttpRequest &req, std::string filePath) {
	std::vector<std::string> env;
	env.push_back("REQUEST_METHOD=" + httpMethodToString(req.method));
	env.push_back("SERVER_PROTOCOL=" + req.version);
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");

	if (req.srv) {
		env.push_back("SERVER_NAME=" + req.srv->host);
		env.push_back("SERVER_PORT=" + toStringInt(req.srv->port));
	}

	//env.push_back("SCRIPT_NAME=" + req.path);
	env.push_back("REMOTE_ADDR=127.0.0.1");
	env.push_back("PATH_INFO=/");
	//env.push_back("PATH_TRANSLATED=" + filePath);
	if (!req.queryString.empty())
		//env.push_back("QUERY_STRING=" + req.queryString);
	if (req.method == POST) {
		if (req.headers.find("content-type") != req.headers.end()) {
			env.push_back("CONTENT_TYPE=" + req.headers["content-type"]);
		}
		if (req.headers.find("content-Length") != req.headers.end()) {
			env.push_back("CONTENT_LENGTH=" + req.headers["content-length"]);
		}
	}

	for (std::map<std::string, std::string>::iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
		std::string key = it->first;
		std::string val = it->second;

		for (size_t i = 0; i < key.length(); ++i) {
			key[i] = toupper(key[i]);
		}

		for (size_t i = 0; i < key.length(); ++i) {
			if (key[i] == '-') {
				key[i] = '_';
			}
		}

		if (key != "CONTENT_TYPE" && key != "CONTENT_LENGTH") {
			env.push_back("HTTP_" + key + "=" + val);
		}
	}

	return env;
}

void exeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd, std::string path) {

	int inputPipe[2];
	int outputPipe[2];

	if (access(path.c_str(), F_OK) == -1 || access(path.c_str(), X_OK) == -1) {
		std::cerr << "Script not found or not executable: " << path << std::endl;
		handle500(res);
		return;
	}

	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1) {
		std::cerr << "pipeError\n";
		handle500(res);
		return;
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cerr << "forkError\n";
		handle500(res);
		return;
	}

	if (pid == 0) {
		close(inputPipe[1]);
		dup2(inputPipe[0], STDIN_FILENO);
		close(inputPipe[0]);

		close(outputPipe[0]);
		dup2(outputPipe[1], STDOUT_FILENO);
		dup2(outputPipe[1], STDERR_FILENO);
		close(outputPipe[1]);
		std::string contentLength = "CONTENT_LENGTH=" + toStringInt(req.content_length);
		std::string pathInfo = "PATH_INFO=" + path;
		std::vector<std::string> env = getEnvp(req, path);
		char **envp = new char *[env.size() + 1];
		for (size_t i = 0; i < env.size(); ++i) {
			char *value = new char [env[i].size() + 1];
			for (size_t j = 0; j < env[i].size(); j++) {
				value[j] = env[i][j];
			}
			value[env[i].size()] = '\0';
			envp[i] = value;
		}
		envp[env.size()] = NULL;

		char *argv[] = { const_cast<char*>(path.c_str()), NULL };
		execve(argv[0], argv, envp);
		exit(1);
	}

	close(inputPipe[0]);
	close(outputPipe[1]);

	serverContext.cgifds[clientFd] = inputPipe[1];
	serverContext.fds[clientFd] = outputPipe[0];
	//Logger::debug("Fd inputPipe %i", serverContext.cgifds[clientFd]);
	serverContext.pids[clientFd] = pid;
	setNonBlocking(serverContext.fds[clientFd]);
	setNonBlocking(serverContext.cgifds[clientFd]);
	res.sendingBodyToCgi = true;
	res.cgiBody = req.body;
	res.statusCode = 200;
	res.statusMessage = "OK";
	res.headers["Content-Type"] = "text/plain";
}

void executeSkript(HttpRequest &req, HttpResponse &res, server &server, int clientFd, file f) {
	//Logger::debug("executer script f.path: %s", f.path.c_str());
	exeSkript(req, res, server.serverContex, clientFd, f.path);
}
