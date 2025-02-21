#include "../../include/webserv.hpp"

void initGET(std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> &get) {
	get["/"] = handleHome;
	get["/assets/img/42Wolfsburg_Logo_ver_pos_black.pdf"] = handleHome;
	get["/assets/img/atoepper"] = handleHome;
	get["/assets/img/jkarras"] = handleHome;
	get["/assets/img/rmatthes"] = handleHome;
	get["/assets/css/index_style.css"] = handleIndexSstyle;
	get["/assets/js/index.js"] = handleHome;
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
