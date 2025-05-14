#include "../../include/webserv.hpp"






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

	for (size_t i = 0; i < directory.dirs.size(); ++i) {
		const dir& subdir = directory.dirs[i];
		std::string name = subdir.path.substr(basePath.size());
		html += indent + "  <li><strong>📁 <a href=\"" + escapeHtml(name) + "/\">" + escapeHtml(name) + "/</a></strong></li>\n";
		buildHtmlForDir(subdir, html, basePath, depth + 1);
	}

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

	std::string filePath = req.path.substr(loc.name.size());
	if (filePath.empty() || filePath[0] == '/') {
		if (loc.index.empty()) {
			filePath = server.root + server.index;
		} else {
			filePath = server.root + loc.index;
		}
	} else {
		filePath = server.root + filePath;
	}

	SearchResult result;
	if (findInDirTree(server.serverContex.tree, filePath, result)) {
		if (result.isDir) {
			if (loc.index.empty()) {
				handleDirResponse(res, result.foundDir, server.index);
			} else {
				handleDirResponse(res, result.foundDir, loc.index);
			}
		} else {
			handleFileResponse(res, result.foundFile.path, result.foundFile.contentType, 200, "OK");
		}
	} else {
		handle404(res);
	}
}

void handleGetWithAnotherRoot(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd) {

	std::string filePath = req.path.substr(loc.name.size());
	if (filePath.empty() || filePath[0] == '/') {
		if (loc.index.empty()) {
			filePath = loc.root + "/" + server.index;
		} else {
			filePath = loc.root + "/" + loc.index;
		}
	} else {
		filePath = loc.root + "/" + filePath;
	}

	SearchResult result;
	if (findInDirTree(loc.tree, filePath, result)) {
		if (result.isDir) {
			if (loc.index.empty()) {
				handleDirResponse(res, result.foundDir, server.index);
			} else {
				handleDirResponse(res, result.foundDir, loc.index);
			}
		} else {
			handleFileResponse(res, result.foundFile.path, result.foundFile.contentType, 200, "OK");
		}
	} else {
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
