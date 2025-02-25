#include "../../include/webserv.hpp"

void routeRequestGET(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.get.find(req.path) != serverContext.get.end()) {
		serverContext.get[req.path](req, res);
	} else {
		std::cout << "asdasdasdas\n";
		handle404(req, res);
	}
}
