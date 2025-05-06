#include "../../include/webserv.hpp"

void exeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd, std::string path) {

	int inputPipe[2];   // Body → Kind
	int outputPipe[2];  // Antwort ← Kind

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
		close(inputPipe[1]);   // Schreibende Seite des Inputs schließen
		dup2(inputPipe[0], STDIN_FILENO);
		close(inputPipe[0]);

		close(outputPipe[0]);  // Lesende Seite des Outputs schließen
		dup2(outputPipe[1], STDOUT_FILENO);
		dup2(outputPipe[1], STDERR_FILENO);
		close(outputPipe[1]);
		// Umgebungsvariablen
		std::string contentLength = "CONTENT_LENGTH=" + toStringInt(req.content_length);
		std::string pathInfo = "PATH_INFO=" + path;
		char *envp[] = {
			const_cast<char*>("REQUEST_METHOD=POST"),
			const_cast<char*>("SERVER_PROTOCOL=HTTP/1.1"),
			const_cast<char*>(contentLength.c_str()),
			const_cast<char*>(pathInfo.c_str()),
			NULL
		};

		char *argv[] = { const_cast<char*>(path.c_str()), NULL };
		execve(argv[0], argv, envp);
		exit(1); // Falls execve fehlschlägt
	}

	// Elternprozess
	close(inputPipe[0]);   // Leseende des Inputs schließen
	close(outputPipe[1]);  // Schreibende Seite des Outputs schließen


	// Output für späteres Auslesen speichern
	serverContext.cgifds[clientFd] = inputPipe[1];
	serverContext.fds[clientFd] = outputPipe[0];
	Logger::debug("Fd inputPipe %i", serverContext.cgifds[clientFd]);
	serverContext.pids[clientFd] = pid;
	setNonBlocking(serverContext.fds[clientFd]);
	setNonBlocking(serverContext.cgifds[clientFd]);
	res.sendingBodyToCgi = true;
	res.cgiBody = req.body;
	// Du kannst hier auf res verzichten oder ihn für Status setzen
	res.statusCode = 200;
	res.statusMessage = "OK";
	res.headers["Content-Type"] = "text/plain";
}


void debugExeSkript(HttpRequest &req, std::string path) {
    int outPipe[2];  // Für stdout vom Child
    int inPipe[2];   // Für stdin vom Child

    if (access(path.c_str(), F_OK) == -1 || access(path.c_str(), X_OK) == -1) {
        std::cerr << "Script nicht vorhanden oder nicht ausführbar: " << path << "\n";
        return;
    }

    if (pipe(outPipe) == -1 || pipe(inPipe) == -1) {
        std::cerr << "pipe Fehler\n";
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "fork Fehler\n";
        return;
    }

    if (pid == 0) {
        // --- Child ---
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(outPipe[1], STDERR_FILENO);

        close(inPipe[1]);
        close(inPipe[0]);
        close(outPipe[0]);
        close(outPipe[1]);

        std::string contentLength = "CONTENT_LENGTH=" + toStringInt(req.content_length);
		std::string pathInfo = "PATH_INFO=" + path;  // oder was auch immer dein Request-Pfad war
        char *envp[] = {
            const_cast<char*>("REQUEST_METHOD=POST"),
            const_cast<char*>("SERVER_PROTOCOL=HTTP/1.1"),
			const_cast<char*>(pathInfo.c_str()),
            const_cast<char*>(contentLength.c_str()),
            NULL
        };

        char *argv[] = { const_cast<char*>(path.c_str()), NULL };
        execve(argv[0], argv, envp);

        std::cerr << "execve fehlgeschlagen\n";
        exit(1);
    }

    // --- Parent ---
    close(inPipe[0]);
    close(outPipe[1]);

    // Request-Body an stdin des Kindprozesses schicken
    if (!req.body.empty()) {
        size_t total = 0;
        while (total < req.body.size()) {
            ssize_t written = write(inPipe[1], req.body.c_str() + total, req.body.size() - total);
            if (written <= 0) break;
            total += written;
        }
    }
    close(inPipe[1]); // stdin schließen, damit das Kind EOF bekommt

    // Ausgabe vom CGI lesen
    char buffer[1024];
    ssize_t bytesRead;
    std::cout << "=== CGI-Antwort beginnt ===" << std::endl;
    while ((bytesRead = read(outPipe[0], buffer, sizeof(buffer))) > 0) {
        std::cout.write(buffer, bytesRead);
    }
    std::cout << "\n=== CGI-Antwort endet ===" << std::endl;

    close(outPipe[0]);
    waitpid(pid, NULL, 0); // Auf Kindprozess warten
}


void handleLs(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd) {
	exeSkript(req, res, serverContext, clientFd, "./server/scripts/ls.sh");
}

void handleLoop(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd) {
	exeSkript(req, res, serverContext, clientFd, "./server/scripts/infinitLoop.sh");
}

void executeSkript(HttpRequest &req, HttpResponse &res, server &server, int clientFd, file f) {
	Logger::debug("executer script f.path: %s", f.path.c_str());
	//debugExeSkript(req, f.path);
	exeSkript(req, res, server.serverContex, clientFd, f.path);
}
