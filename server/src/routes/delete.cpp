#include "../../include/webserv.hpp"

void routeRequestDELETE(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.del.find(req.path) != serverContext.del.end()) {
		serverContext.del[req.path](req, res);
	} else {
		handle404(req, res);
	}
}
