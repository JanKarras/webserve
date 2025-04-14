#include "../../include/webserv.hpp"

bool isPathSafe(const std::string &fileName) {
	if (fileName.find("..") != std::string::npos) {
		return false;  // Verhindert Pfad-Traversal
	}
	return true;
}

bool isCGIFile(const std::string &fileName, const std::vector<std::string> &cgi_ext) {
	Logger::debug("is File: %s is an exectuabale?", fileName.c_str());
	size_t dotPos = fileName.rfind('.');
	if (dotPos == std::string::npos) {
		return false;
	}
	std::string fileExt = fileName.substr(dotPos);
	Logger::debug("dot detected fileExt: %s -- cgi_ext.size = %i", fileExt.c_str(), cgi_ext.size());
	for (size_t i = 0; i < cgi_ext.size(); ++i) {
		Logger::debug("cgi_ext: %s", cgi_ext[i].c_str());
		if (fileExt == cgi_ext[i]) {
			return true;
		}
	}
	return false;
}

void printFormData(formData &data) {
    Logger::debug("boundary: %s", data.boundary.c_str());
    Logger::debug("Content-Disposition: %s", data.dis.c_str());
    Logger::debug("name: %s", data.name.c_str());
    Logger::debug("filename: %s", data.filename.c_str());
    Logger::debug("Content-Type: %s", data.contentType.c_str());
    Logger::debug("fileContent:\n%s", data.fileContent.c_str());
}

bool parseFormData(HttpRequest &req, HttpResponse &res, formData &formData) {
    // 1. Extract boundary from the header
    std::map<std::string, std::string>::iterator it = req.headers.find("content-type");
    if (it == req.headers.end()) {
        Logger::error("Error: No Content-Type header found.");
        handle500(res);
        return false;
    }

    std::string contentType = it->second;
    std::string boundaryPrefix = "boundary=";
    size_t pos = contentType.find("multipart/form-data");

    if (pos == std::string::npos) {
        Logger::error("Error: Content-Type is not multipart/form-data.");
        handle500(res);
        return false;
    }

    pos = contentType.find(boundaryPrefix);
    if (pos == std::string::npos) {
        Logger::error("Error: No boundary found in Content-Type.");
        handle500(res);
        return false;
    }

    formData.boundary = "--" + contentType.substr(pos + boundaryPrefix.length());

    // 2. Split body into sections
    std::string body = req.body;
    size_t start = body.find(formData.boundary);
    if (start == std::string::npos) {
        Logger::error("Error: Boundary not found in the body.");
        handle500(res);
        return false;
    }
    start += formData.boundary.length(); // Move past the boundary

    // 3. Parse headers
    size_t end = body.find("\r\n\r\n", start);
    if (end == std::string::npos) {
        Logger::error("Error: No double CRLF found (Headers missing or malformed).");
        handle500(res);
        return false;
    }

    std::string headers = body.substr(start, end - start);
    start = end + 4; // Move past CRLF

    // Find Content-Disposition
    pos = headers.find("Content-Disposition:");
    if (pos == std::string::npos) {
        Logger::error("Error: Content-Disposition header missing.");
        handle500(res);
        return false;
    }

    // Extract name
    pos = headers.find("name=\"", pos);
    if (pos == std::string::npos) {
        Logger::error("Error: Name parameter missing in Content-Disposition.");
        handle500(res);
        return false;
    }
    size_t nameStart = pos + 6;
    size_t nameEnd = headers.find("\"", nameStart);
    formData.name = headers.substr(nameStart, nameEnd - nameStart);

    // Extract filename (optional)
    pos = headers.find("filename=\"", nameEnd);
    if (pos != std::string::npos) {
        size_t fileStart = pos + 10;
        size_t fileEnd = headers.find("\"", fileStart);
        formData.filename = headers.substr(fileStart, fileEnd - fileStart);
    }

    // Extract Content-Type (optional)
    pos = headers.find("Content-Type:");
    if (pos != std::string::npos) {
        size_t typeStart = pos + 14;
        size_t typeEnd = headers.find("\r\n", typeStart);
        formData.contentType = headers.substr(typeStart, typeEnd - typeStart);
    }

    // 4. Extract file content
    end = body.find(formData.boundary, start);
    if (end == std::string::npos) {
        Logger::error("Error: No closing boundary found.");
        handle500(res);
        return false;
    }

    formData.fileContent = body.substr(start, end - start - 2); // -2 to remove \r\n before boundary

    printFormData(formData);
    return true;
}

void handleRegularLocation(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd) {
	std::string path = loc.root + "/" + loc.index;
	for (size_t i = 0; i < loc.files.size(); i++) {
		if (path == loc.files[i].path) {
			executeSkript(req, res, server, clientFd, loc.files[i]);
			return ;
		}
	}
	handle404(res);
}

void routeRequestPOST(HttpRequest &req, HttpResponse &res, server &server, location &loc, int clientFd) {
	if (loc.regularLocation) {
		handleRegularLocation(req, res, server, loc, clientFd);
		return;
	}
	std::string uploadDir;

	std::cout << req.body << std::endl;

	if (loc.root.size() != 0) {
		uploadDir = loc.root;
	} else {
		uploadDir = server.root;
	}

	formData form;

	if (parseFormData(req, res, form) == false) {
		return;
	}

	if (uploadDir.empty()) {
		Logger::error("Upload directory is not set.");
		handle500(res);
		return;
	}

	std::string fileName = form.filename;
	if (fileName.empty() || !isPathSafe(fileName)) {
		Logger::error("Invalid or unsafe filename");
		handle400(res);
		return;
	}

	std::string filePath = uploadDir + fileName;
	std::ofstream outfile(filePath.c_str(), std::ios::binary);

	if (!outfile) {
		Logger::error("Failed to open file for writing: %s", filePath.c_str());
		handle500(res);
		return;
	}

	outfile.write(form.fileContent.c_str(), form.fileContent.size());
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

	std::cout << "newFile: " << newFile.path << " -- " << newFile.contentType << std::endl;

	if (loc.root.empty()) {
		server.serverContex.files.push_back(newFile);
	} else {
		loc.files.push_back(newFile);
	}

	Logger::info("File uploaded successfully: %s", filePath.c_str());
	res.statusCode = 201;
	res.statusMessage = "Created";
	res.body = "File uploaded successfully.";

}
