#include "../../include/webserv.hpp"

bool isPathSafe(const std::string &fileName) {
	if (fileName.find("..") != std::string::npos) {
		return false;  // Verhindert Pfad-Traversal
	}
	return true;
}

bool isCGIFile(const std::string &fileName, const std::vector<std::string> &cgi_ext) {
	size_t dotPos = fileName.rfind('.');
	if (dotPos == std::string::npos) {
		return false;
	}

	std::string fileExt = fileName.substr(dotPos);
	for (size_t i = 0; i < cgi_ext.size(); ++i) {
		if (fileExt == cgi_ext[i]) {
			return true;
		}
	}
	return false;
}

void routeRequestPOST(HttpRequest &req, HttpResponse &res, server &server, location &loc) {
	std::string uploadDir;

	if (loc.root.size() != 0) {
		uploadDir = loc.root;
	} else {
		uploadDir = server.root;
	}

	if (uploadDir.empty()) {
		Logger::error("Upload directory is not set.");
		handle500(res);
		return;
	}

	if (req.body.empty()) {
		Logger::error("No file data received.");
		handle400(res);
		return;
	}

	std::string fileName = req.headers["Filename"];
	if (fileName.empty() || !isPathSafe(fileName)) {
		Logger::error("Invalid or unsafe filename");
		handle400(res);
		return;
	}

	std::string filePath = uploadDir + "/" + fileName;
	std::ofstream outfile(filePath.c_str(), std::ios::binary);

	if (!outfile) {
		Logger::error("Failed to open file for writing: %s", filePath.c_str());
		handle500(res);
		return;
	}

	outfile.write(req.body.c_str(), req.body.size());
	outfile.close();

	file newFile;

	newFile.path = filePath;
	newFile.contentType = getContentType(getFileExtension(filePath));

	if (isCGIFile(fileName, loc.cgi_ext)) {
		Logger::info("Making CGI script executable: %s", filePath.c_str());

		if (!setsetExecutable(filePath)) {
			Logger::error("Failed to set file as executable: %s", filePath.c_str());
		}
	}

	std::cout << "newFile: " << newFile.path << "--" << newFile.contentType << std::endl;

	Logger::info("File uploaded successfully: %s", filePath.c_str());
	res.statusCode = 201;
	res.statusMessage = "Created";
	res.body = "File uploaded successfully.";

}
