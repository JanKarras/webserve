#include "../../include/webserv.hpp"

void routeRequestGET(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (req.cgi && serverContext.cgi.find(req.path) != serverContext.cgi.end()) {
		serverContext.cgi[req.path](req, res);
	} else if (serverContext.get.find(req.path) != serverContext.get.end()) {
		serverContext.get[req.path](req, res);
	} else if (serverContext.pages.find(req.path) != serverContext.pages.end()) {
		serverContext.pages[req.path](res);
	} else {
		handle404(res);
	}
}
