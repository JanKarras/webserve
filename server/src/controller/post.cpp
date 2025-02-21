#include "../../include/webserv.hpp"

void handleLogin(HttpRequest &req, HttpResponse &res) {
	if (req.headers["Content-Type"] != "application/json") {
		handle400(req, res);
		return;
	}
	
}
