#include "../../include/webserv.hpp"

void routeRequestGET(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.get.find(req.path) != serverContext.get.end()) {
		serverContext.get[req.path](req, res);
	} else if (serverContext.pages.find(req.path) != serverContext.pages.end()) {
		serverContext.pages[req.path](res);
	} else {
		handle404(res);
	}
}
