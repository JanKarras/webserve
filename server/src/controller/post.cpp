#include "../../include/webserv.hpp"

void handleLogin(HttpRequest &req, HttpResponse &res) {
	if (req.headers["Content-Type"] != "application/json") {
		handle400(req, res);
		return;
	}
	std::ifstream file("server/src/db/loginData.csv");
	std::map<std::string, std::string> data = parseSimpleJSON(req.body);

	if(data.find("email") == data.end() || data.find("password") == data.end()) {
		handle400(req, res);
		return;
	}

	std::string email = data["email"];
	std::string password = data["password"];

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

void handleCreateAccount(HttpRequest &req, HttpResponse &res) {
	if (req.headers["Content-Type"] != "application/json") {
		handle400(req, res);
		return;
	}

	std::map<std::string, std::string> data = parseSimpleJSON(req.body);

	if (data.find("email") == data.end() || data.find("password") == data.end()) {
		handle400(req, res);
		return;
	}

	std::string email = data["email"];
	std::string password = data["password"];

	std::ifstream file("server/src/db/loginData.csv");
	if (!file) {
		std::cerr << "File not found\n";
		handle404(req, res);
		return;
	}

	std::string line;
	while (getline(file, line)) {
		std::stringstream ss(line);
		std::string emailDb, passwordDb;

		if (getline(ss, emailDb, '|') && getline(ss, passwordDb, '|')) {
			if (emailDb == email) {
				res.statusCode = 409;
				res.statusMessage = "Conflict";
				res.body = "Account already exists";
				res.headers["Content-Type"] = "application/json";
				file.close();
				return;
			}
		} else {
			std::cerr << "Error parsing line: " << line << "\n";
			handle500(req, res);
			file.close();
			return;
		}
	}
	file.close();

	std::ofstream outFile("server/src/db/loginData.csv", std::ios::app);
	if (!outFile) {
		std::cerr << "Error opening file for writing\n";
		handle500(req, res);
		return;
	}

	outFile << email << "|" << password << "\n";
	outFile.close();

	res.statusCode = 201;
	res.statusMessage = "Created";
	res.body = "Account created successfully";
	res.headers["Content-Type"] = "application/json";
}
