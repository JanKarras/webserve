#include "../../include/webserv.hpp"


std::string stripTrailingSlash(const std::string &path) {
	if (path.length() > 1 && path[path.length() - 1] == '/')
		return path.substr(0, path.length() - 1);
	return path;
}

bool findInDirTree(const dir &current, const std::string &targetPath, SearchResult &result) {
	std::string normalizedTarget = stripTrailingSlash(targetPath);

	// Check current dir itself
	if (current.path == normalizedTarget) {
		result.found = true;
		result.isDir = true;
		result.foundDir = current;
		return true;
	}

	// Check files in current dir
	for (size_t i = 0; i < current.files.size(); ++i) {
		if (current.files[i].path == normalizedTarget) {
			result.found = true;
			result.isDir = false;
			result.foundFile = current.files[i];
			return true;
		}
	}

	// Recursively check subdirectories
	for (size_t i = 0; i < current.dirs.size(); ++i) {
		if (findInDirTree(current.dirs[i], normalizedTarget, result)) {
			return true;
		}
	}

	return false;
}



std::string escapeHtml(const std::string& input) {
	std::string escaped;
	for (size_t i = 0; i < input.length(); ++i) {
		switch (input[i]) {
			case '&': escaped += "&amp;"; break;
			case '<': escaped += "&lt;"; break;
			case '>': escaped += "&gt;"; break;
			case '"': escaped += "&quot;"; break;
			case '\'': escaped += "&#39;"; break;
			default: escaped += input[i];
		}
	}
	return escaped;
}

void buildHtmlForDir(const dir& directory, std::string& html, const std::string& basePath, int depth = 0) {
	std::string indent(depth * 2, ' ');

	html += indent + "<ul>\n";

	// Unterverzeichnisse
	for (size_t i = 0; i < directory.dirs.size(); ++i) {
		const dir& subdir = directory.dirs[i];
		std::string name = subdir.path.substr(basePath.size());
		html += indent + "  <li><strong>📁 <a href=\"" + escapeHtml(name) + "/\">" + escapeHtml(name) + "/</a></strong></li>\n";
		buildHtmlForDir(subdir, html, basePath, depth + 1);
	}

	// Dateien
	for (size_t i = 0; i < directory.files.size(); ++i) {
		const file& f = directory.files[i];
		std::string name = f.path.substr(basePath.size());
		html += indent + "  <li>📄 <a href=\"" + escapeHtml(name) + "\">" + escapeHtml(name) + "</a></li>\n";
	}

	html += indent + "</ul>\n";
}

std::string generateAutoindexHtml(const dir& directory) {
	std::string html;
	html += "<!DOCTYPE html>\n";
	html += "<html><head>\n";
	html += "<meta charset=\"UTF-8\">\n";  // <--- Das ist neu
	html += "<title>Index of " + escapeHtml(directory.path) + "</title>\n";
	html += "</head><body>\n";
	html += "<h1>Index of " + escapeHtml(directory.path) + "</h1>\n";

	buildHtmlForDir(directory, html, directory.path);

	html += "</body></html>\n";
	return html;
}

std::string getFilename(const std::string& fullPath) {
	size_t pos = fullPath.find_last_of('/');
	if (pos == std::string::npos) return fullPath;
	return fullPath.substr(pos + 1);
}


bool containsIndexInCurrentDir(const dir& d, const std::string& indexName) {
	for (size_t i = 0; i < d.files.size(); ++i) {
		if (getFilename(d.files[i].path) == indexName) {
			return true;
		}
	}
	return false;
}

void handleDirResponse(HttpResponse &res, dir directory, std::string index) {
	if (!containsIndexInCurrentDir(directory, index)) {
		handle404(res);
		return;
	}
	res.statusMessage = "OK";
	res.headers["Content-Type"] = "text/html";
	res.statusCode = 200;
	res.body = generateAutoindexHtml(directory);
	res.headers["Content-Length"] = toStringInt(res.body.size());
}

void handleGet(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd) {
	Logger::debug("Get with the server root called for loc: %s with path: %s", loc.name.c_str(), req.path.c_str());

	std::string filePath = req.path.substr(loc.name.size());
	if (filePath.empty() || filePath[0] == '/') {
		Logger::debug("Empty or only / path detected");
		if (loc.index.empty()) {
			filePath = server.root + server.index;
		} else {
			filePath = server.root + loc.index;
		}
	} else {
		filePath = server.root + filePath;
	}


	Logger::debug("Requested relative file path: %s", filePath.c_str());

	SearchResult result;
	if (findInDirTree(server.serverContex.tree, filePath, result)) {
		if (result.isDir) {
			Logger::debug("📁 Directory found: %s", result.foundDir.path.c_str());
			if (loc.index.empty()) {
				handleDirResponse(res, result.foundDir, server.index);
			} else {
				handleDirResponse(res, result.foundDir, loc.index);
			}
		} else {
			Logger::debug("📄 File found: %s (Content-Type: %s)", result.foundFile.path.c_str(), result.foundFile.contentType.c_str());
			handleFileResponse(res, result.foundFile.path, result.foundFile.contentType, 200, "OK");
		}
	} else {
		Logger::debug("❌ Nothing found for: %s", filePath.c_str());
		handle404(res);
	}
}

void handleGetWithAnotherRoot(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd) {
	Logger::debug("Get with another root called for loc: %s with path: %s", loc.name.c_str(), req.path.c_str());

	std::string filePath = req.path.substr(loc.name.size());
	if (filePath.empty() || filePath[0] == '/') {
		Logger::debug("Empty or only / path detected");
		if (loc.index.empty()) {
			filePath = loc.root + "/" + server.index;
		} else {
			filePath = loc.root + "/" + loc.index;
		}
	} else {
		filePath = loc.root + "/" + filePath;
	}


	Logger::debug("Requested relative file path: %s", filePath.c_str());

	SearchResult result;
	if (findInDirTree(loc.tree, filePath, result)) {
		if (result.isDir) {
			Logger::debug("📁 Directory found: %s", result.foundDir.path.c_str());
			if (loc.index.empty()) {
				handleDirResponse(res, result.foundDir, server.index);
			} else {
				handleDirResponse(res, result.foundDir, loc.index);
			}
		} else {
			Logger::debug("📄 File found: %s (Content-Type: %s)", result.foundFile.path.c_str(), result.foundFile.contentType.c_str());
			handleFileResponse(res, result.foundFile.path, result.foundFile.contentType, 200, "OK");
		}
	} else {
		Logger::debug("❌ Nothing found for: %s", filePath.c_str());
		handle404(res);
	}
}

void routeRequestGET(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd) {
	if (loc.root.empty()) {
		handleGet(req, res, server, loc, clientFd);
	} else {
		handleGetWithAnotherRoot(req, res, server, loc, clientFd);
	}
}
