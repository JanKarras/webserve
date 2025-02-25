#include "../../include/webserv.hpp"

void delteFile(HttpRequest &req, HttpResponse &res) {
	std::string email = req.query["email"];
	std::string fileName = req.query["fileName"];

	if (email.empty() || fileName.empty()) {
		handle400(req, res);
		return;
	}

	std::string path = getDestPath(email);

	if (email.empty()) {
		handle404(req, res);
		return;
	}

	path.append("/" + fileName);

	if (remove(path.c_str()) != 0) {
		std::cerr << "Fehler beim Löschen der Datei: " << std::endl;
		handle500(req, res);
		return;
	}
	
	res.statusCode = 200;
	res.statusMessage = "OK";
	res.body = "File deleted successfully";
	res.headers["Content-Type"] = "application/json";
}
