#include "../../include/webserv.hpp"

void routeRequestPOST(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.post.find(req.uri) != serverContext.post.end()) {
		serverContext.post[req.uri](req, res);
	} else {
		handle404(req, res);
	}
}
