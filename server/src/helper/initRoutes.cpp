#include "../../include/webserv.hpp"

void initGET(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &get) {
	get.clear();
}

void initPOST(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &post) {
	post["/auth/login"] = handleLogin;
}

void initDEL(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &del) {
	del.clear();
}

void initRoutes(ServerContext &serverContext) {
	initGET(serverContext.get);
	initPOST(serverContext.post);
	initDEL(serverContext.del);
}
