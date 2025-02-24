#include "../../include/webserv.hpp"

void routeRequestGET(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.get.find(req.uri) != serverContext.get.end()) {
		serverContext.get[req.uri](req, res);
	} else {
		std::cout << "asdasdasdas\n";
		handle404(req, res);
	}
}
