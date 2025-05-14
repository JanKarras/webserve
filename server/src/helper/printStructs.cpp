#include "../../include/webserv.hpp"


void printHttpResponse(const HttpResponse &res) {
	std::cout << "HttpResponse:\n";
	std::cout << "  version: " << res.version << "\n";
	std::cout << "  statusCode: " << res.statusCode << "\n";
	std::cout << "  statusMessage: " << res.statusMessage << "\n";
	std::cout << "  headers:\n";
	std::map<std::string, std::string>::const_iterator it;
	for (it = res.headers.begin(); it != res.headers.end(); ++it)
		std::cout << "    " << it->first << ": " << it->second << "\n";
	std::cout << "  body: " << res.body << "\n";
	std::cout << "  state: " << res.state << "\n";
	std::cout << "  bodySent: " << res.bodySent << "\n";
	std::cout << "  CgiBodySent: " << res.CgiBodySent << "\n";
	std::cout << "  startTime: " << res.startTime << "\n";
	std::cout << "  sendingBodyToCgi: " << res.sendingBodyToCgi << "\n";
	std::cout << "  readFromCgiFinished: " << res.readFromCgiFinished << "\n";
	std::cout << "  writeError: " << res.writeError << "\n";
	std::cout << "  chunked: " << res.chunked << "\n";
	std::cout << "  cgiBody: " << res.cgiBody << "\n";
	std::cout << "  cgiBodySize: " << res.cgiBodySize << "\n";
}

void printFile(const file &f) {
	std::cout << "File:\n";
	std::cout << "  contentType: " << f.contentType << "\n";
	std::cout << "  path: " << f.path << "\n";
}

void printDir(const dir &d, int indent) {
	std::string prefix(indent, ' ');
	std::cout << prefix << "Dir: " << d.path << "\n";
	for (size_t i = 0; i < d.dirs.size(); ++i)
		printDir(d.dirs[i], indent + 2);
	for (size_t i = 0; i < d.files.size(); ++i)
		std::cout << prefix << "  File: " << d.files[i].path << " (" << d.files[i].contentType << ")\n";
}

void printSearchResult(const SearchResult &res) {
	std::cout << "SearchResult:\n";
	std::cout << "  found: " << res.found << "\n";
	std::cout << "  isDir: " << res.isDir << "\n";
	if (res.isDir) printDir(res.foundDir);
	else printFile(res.foundFile);
}

void printErrorPage(const errorPage &e) {
	std::cout << "ErrorPage:\n";
	std::cout << "  errorCode: " << e.errorCode << "\n";
	std::cout << "  path: " << e.path << "\n";
}

void printLocation(const location &loc) {
	std::cout << "Location:\n";
	std::cout << "  name: " << loc.name << "\n";
	std::cout << "  root: " << loc.root << "\n";
	std::cout << "  index: " << loc.index << "\n";
	std::cout << "  del/get/post: " << loc.del << "/" << loc.get << "/" << loc.post << "\n";
	std::cout << "  client_max_body_size: " << loc.client_max_body_size << "\n";
	std::cout << "  redirect: " << loc.redirect << "\n";
	std::cout << "  regularLocation: " << loc.regularLocation << "\n";
	std::cout << "  ext: " << loc.ext << "\n";
	std::cout << "  pattern: " << loc.pattern << "\n";
	std::cout << "  cgi_paths: ";
	for (size_t i = 0; i < loc.cgi_paths.size(); ++i)
		std::cout << loc.cgi_paths[i] << " ";
	std::cout << "\n  cgi_ext: ";
	for (size_t i = 0; i < loc.cgi_ext.size(); ++i)
		std::cout << loc.cgi_ext[i] << " ";
	std::cout << "\n";
	printDir(loc.tree, 2);
}

void printServer(const server &srv) {
	std::cout << "Server:\n";
	std::cout << "  port: " << srv.port << "\n";
	std::cout << "  host: " << srv.host << "\n";
	std::cout << "  server_name: " << srv.server_name << "\n";
	std::cout << "  root: " << srv.root << "\n";
	std::cout << "  index: " << srv.index << "\n";
	std::cout << "  client_max_body_size: " << srv.client_max_body_size << "\n";
	std::cout << "  errorPages:\n";
	for (size_t i = 0; i < srv.errorpages.size(); ++i)
		printErrorPage(srv.errorpages[i]);
	std::cout << "  locations:\n";
	for (size_t i = 0; i < srv.locations.size(); ++i)
		printLocation(srv.locations[i]);
	std::cout << "  tree:\n";
	printDir(srv.serverContex.tree);
}

void printHttpRequest(const HttpRequest &req) {
	std::cout << "HttpRequest:\n";
	std::cout << "  method: " << req.method << "\n";
	std::cout << "  uri: " << req.uri << "\n";
	std::cout << "  version: " << req.version << "\n";
	std::cout << "  path: " << req.path << "\n";
	std::cout << "  queryString: " << req.queryString << "\n";
	std::cout << "  headers:\n";
	std::map<std::string, std::string>::const_iterator it;
	for (it = req.headers.begin(); it != req.headers.end(); ++it)
		std::cout << "    " << it->first << ": " << it->second << "\n";
	std::cout << "  body: " << req.body << "\n";
	std::cout << "  content_length: " << req.content_length << "\n";
	std::cout << "  parseState: " << req.parseState << "\n";
	std::cout << "  state: " << req.state << "\n";
	std::cout << "  exitStatus: " << req.exitStatus << "\n";
	std::cout << "  clientFd: " << req.clientFd << "\n";
	std::cout << "  startTime: " << req.startTime << "\n";
}

void printConfigData(const ConfigData &config) {
	std::cout << "ConfigData:\n";
	std::cout << "  epollFd: " << config.epollFd << "\n";
	std::cout << "  serverFd: " << config.serverFd << "\n";
	std::cout << "  port: " << config.port << "\n";
	std::cout << "  requests:\n";
	std::map<int, HttpRequest>::const_iterator it;
	for (it = config.requests.begin(); it != config.requests.end(); ++it)
		printHttpRequest(it->second);
	std::cout << "  servers:\n";
	for (size_t i = 0; i < config.servers.size(); ++i)
		printServer(config.servers[i]);
}

void printServerContext(const ServerContext &ctx) {
	std::cout << "ServerContext:\n";
	std::cout << "  requests:\n";
	std::map<int, HttpRequest*>::const_iterator it_req;
	for (it_req = ctx.requests.begin(); it_req != ctx.requests.end(); ++it_req) {
		std::cout << "    fd " << it_req->first << ":\n";
		printHttpRequest(*it_req->second);
	}
	std::cout << "  responses:\n";
	std::map<int, HttpResponse>::const_iterator it_res;
	for (it_res = ctx.responses.begin(); it_res != ctx.responses.end(); ++it_res) {
		std::cout << "    fd " << it_res->first << ":\n";
		printHttpResponse(it_res->second);
	}
	std::cout << "  tree:\n";
	printDir(ctx.tree);
}
