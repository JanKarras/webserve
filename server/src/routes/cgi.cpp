#include "../../include/webserv.hpp"

void routeRequestCGI(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd) {
	if (serverContext.cgi.find(req.path) != serverContext.cgi.end()) {
		serverContext.cgi[req.path](req, res, serverContext, clientFd);
	} else {
		handle404(res);
	}
}
