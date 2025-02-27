#include "../../include/webserv.hpp"

void handleGetFile(HttpRequest &req, HttpResponse &res) {
	std::string email = req.query["email"];
	std::string fileName = req.query["fileName"];

	if (email.empty() || fileName.empty()) {
		handle400(res);
		return;
	}

	std::string path = getDestPath(email);

	if (path.length() == 0) {
		handle500(res);
		return ;
	}

	path.append("/" + fileName);

	res.body = getFileContent(path);

	if (res.body.size() == 0) {
		handle404(res);
		return ;
	}

	std::string fileExtension = fileName.substr(fileName.find_last_of(".") + 1);

	std::map<std::string, std::string> mimeTypes = initMimeTypes();

	if (mimeTypes.find(fileExtension) != mimeTypes.end()) {
		res.headers["Content-Type"] = mimeTypes[fileExtension];
	} else {
		res.headers["Content-Type"] = "application/octet-stream";
	}
	res.statusCode = 200;
	res.statusMessage = "File downloade succesfuly";
}

void getFileNames(HttpRequest &req, HttpResponse &res) {
	std::string email = req.query["email"];

	if (email.empty()) {
		handle400(res);
		return;
	}

	std::string path = getDestPath(email);

	if (path.length() == 0) {
		handle500(res);
		return ;
	}

	DIR *dir = opendir(path.c_str());

	if (!dir) {
		handle404(res);
		return;
	}

	struct dirent *entry;
	std::string fileList;

	while ((entry = readdir(dir)) != NULL){
		std::string fileName = entry->d_name;
		if (fileName != "." && fileName != "..") {
			if (!fileList.empty()) {
				fileList += ";";
			}
			fileList += fileName;
		}
	}
	closedir(dir);

	res.statusCode = 200;
	res.statusMessage = "OK";
	res.body = fileList;
	res.headers["Content-Type"] = "text/plain";
}
