#include "../../include/webserv.hpp"

void routeRequestDELETE(HttpRequest &req, HttpResponse &res) {
	if (req.uri == "") {

    } else {
        handleNotFound(req, res);
    }
}
