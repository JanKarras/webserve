#include "../../include/webserv.hpp"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <unistd.h>

std::string stripTrailingSlashDel(const std::string &path) {
	if (path.length() > 1 && path[path.length() - 1] == '/')
		return path.substr(0, path.length() - 1);
	return path;
}

bool findInDirTreeDel(const dir &current, const std::string &targetPath, SearchResult &result) {
	std::string normalizedTarget = stripTrailingSlashDel(targetPath);
	if (current.path == normalizedTarget) {
		result.found = true;
		result.isDir = true;
		result.foundDir = current;
		return true;
	}

	for (size_t i = 0; i < current.files.size(); ++i) {
		if (current.files[i].path == normalizedTarget) {
			result.found = true;
			result.isDir = false;
			result.foundFile = current.files[i];
			return true;
		}
	}

	for (size_t i = 0; i < current.dirs.size(); ++i) {
		if (findInDirTreeDel(current.dirs[i], normalizedTarget, result)) {
			return true;
		}
	}

	return false;
}

void deleteFileFromTree(dir &tree, std::string filePath) {
	for (size_t i = 0; i < tree.files.size(); i++) {
		file &f = tree.files[i];
		if (filePath == f.path) {
			tree.files.erase(tree.files.begin() + i);
			return;
		}
	}
	for (size_t i = 0; i < tree.dirs.size(); i++) {
		dir &d = tree.dirs[i];
		deleteFileFromTree(d, filePath);
	}

}

void deleteFile(SearchResult &result, HttpResponse &res, dir &tree) {
	if (access(result.foundFile.path.c_str(), F_OK) == -1) {
		handle404(res);
		return;
	}

	if (access(result.foundFile.path.c_str(), W_OK) == -1) {
		handle403(res);
		return;
	}

	pid_t pid;

	pid = fork();
	if (pid == 0) {
		char *envp[] = {
			NULL
		};
		char *args[] = {
			(char *)"rm",
			(char *)"-f",
			(char *)result.foundFile.path.c_str(),
			NULL
		};
		if (execve("/bin/rm", args, envp) == -1) {
			Logger::error("Faild to execute rm to delte %s", result.foundFile.path.c_str());
			exit(1);
		}
	} else {
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			deleteFileFromTree(tree, result.foundFile.path);
			res.statusCode = 200;
			res.body = "File " + result.foundFile.path + " deleted";

		} else {
			handle500(res);
		}
	}
}

void routeRequestDELETE(HttpRequest &req, HttpResponse &res, server &server, location &loc) {
	std::string filePath = req.path.substr(loc.name.size());

	dir &tree = loc.root.empty() ? server.serverContex.tree : loc.tree;

	if ((filePath.empty() || filePath[filePath.length() - 1] == '/') && !loc.index.empty()) {
		filePath = joinPath(filePath, loc.index);
	}

	if (loc.root.empty()) {
		filePath = joinPath(server.root, filePath);
	} else {
		filePath = joinPath(loc.root, filePath);
	}

	SearchResult result;

	if (findInDirTreeDel(tree, filePath, result)) {
		printSearchResult(result);
		if (result.isDir) {
			handle403(res);
		} else {
			deleteFile(result, res, tree);
		}
	} else {
		handle404(res);
	}
}


