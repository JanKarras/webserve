#include "../../include/webserv.hpp"

void handleLs(HttpRequest &req, HttpResponse &res) {
	res.statusCode = req.exitStatus;
	std::cout << "cgi" << std::endl;
}
