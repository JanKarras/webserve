#include "../../include/webserv.hpp"

void handleLogin(HttpRequest &req, HttpResponse &res) {
	if (req.headers["Content-Type"] != "application/json") {
		handle400(req, res);
		return;
	}
	std::ifstream file("../db/loginData.csv");

	std::string email;
	std::string password;

	if (file) {
		std::string line;
		while (getline(file, line)) {
			std::stringstream ss(line);
			std::string emailDb, passwordDb;
			if (getline(ss, emailDb, '|') && getline(ss, passwordDb, '|')) {
				if (emailDb == email && password == passwordDb) {
					res.statusCode = 200;
					res.statusMessage = "OK";
					res.body = std::string("login succesfull");
					res.headers["Content-Type"] = "application/json";
					file.close();
					return ;
				}
			} else {
				std::cerr << "Error parsing line: " << line << "\n";
				handle500(req, res);
				file.close();
				return;
			}
		}
		handle401(req, res);
		file.close();
	} else {
		std::cerr << "File not found\n";
		handle404(req, res);
	}
}
