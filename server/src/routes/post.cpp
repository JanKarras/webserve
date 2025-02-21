#include "../../include/webserv.hpp"

void routeRequestPOST(HttpRequest &req, HttpResponse &res) {
    if (req.uri == "/auth/login") {
        handleLogin(req, res);
    } else {
        handleNotFound(req, res);
    }
}
