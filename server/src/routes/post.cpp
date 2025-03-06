#include "../../include/webserv.hpp"

void routeRequestPOST(HttpRequest &req, HttpResponse &res, ServerContext serverContext) {
	if (serverContext.post.find(req.path) != serverContext.post.end()) {
		serverContext.post[req.path](req, res);
	} else {
		handle404(res);
	}
}
