#include "../../include/webserv.hpp"

void routeRequestDELETE(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.del.find(req.uri) != serverContext.del.end()) {
		serverContext.del[req.uri](req, res);
	} else {
		handle404(req, res);
	}
}
