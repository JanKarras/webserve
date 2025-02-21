#include "../../include/webserv.hpp"

void routeRequestGET(HttpRequest &req, HttpResponse &res) {
	if (req.uri == "/") {
        handleLogin(req, res);
    } else {
        handle404(req, res);
    }
}
