#ifndef WEBSERV_HPP

#define WEBSERV_HPP

#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <map>
#include <sstream>
#include <fstream>
#include <cerrno>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include "http_request.hpp"
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>

#define CR (u_char) 'r'
#define LF (u_char) 'n'

#define SUCCESS 0
#define FAILURE 1

#define MAX_EVENTS 1024
#define MAX_CLIENTS 1024
#define PORT 8080
#define CHUNK_SIZE 1024
#define BUFFER_SIZE 1024
#define TIME_TO_KILL_CHILD 10

#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_N_ALLOWED 405
#define HTTP_ENTITY_TOO_LARGE 413
#define HTTP_SERVER_ERROR 500


extern bool running;

enum ResponseState {
	NOT_STARTED,
	SENDING_HEADERS,
	SENDING_BODY,
	RESP_COMPLETE
};

struct HttpResponse {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;
	ResponseState state;
	size_t bodySent;
	long long startTime;

	HttpResponse() : version("HTTP/1.1"), statusCode(200), statusMessage("OK"), state(NOT_STARTED), bodySent(0), startTime(0) {}
};

struct ServerContext {
	int serverFd;
	int epollFd;
	std::map<int, HttpRequest> requests;
	std::map<int, HttpResponse> responses;
	std::map<int, int> fds;
	std::map<int, pid_t> pids;
	std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> get;
	std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> post;
	std::map<std::string, void (*)(HttpRequest &, HttpResponse &)> del;
	std::map<std::string, void (*)(HttpRequest &, HttpResponse &, ServerContext &, int)> cgi;
	std::map<std::string, void (*)(HttpResponse &)> pages;
};

struct ConficServer {

};


struct ConficData {
	ConficServer *server;
	int nb;
};


//CONFIC
bool parseConfic(std::string path, ConficData *data);

//HTTPPARSER
void printHttpRequest(const HttpRequest& request);
void parseHttpRequest(HttpRequest &req, std::string &data);

//SERVER
void startServer(ConficData &conficData, bool conficFlag);
bool initServer(ServerContext &ServerContext, struct sockaddr_in &serverAddress, struct epoll_event &event);
bool initServerConfic(ServerContext &ServerContext, struct sockaddr_in &serverAddress, struct epoll_event &event, ConficData &conficData);
bool addEvent(ServerContext &ServerContext, struct epoll_event &event);
bool handleEventReq(ServerContext &ServerContext, struct epoll_event *events, int i);
bool handleEventRes(ServerContext &ServerContext, struct epoll_event *events, int i);
//SIG
void handle_sigint(int sig, siginfo_t *siginfo, void *context);
bool initSignal(void);
//HANDLE REQ
void handleRequest(int clientFd, ServerContext &ServerContext);
void handleErrorRequest(int clientFd, ServerContext &ServerContext);
//helper
std::string toStringInt(int number);
int toIntString(const std::string &str);
std::string toString(long long number);
std::string getDestPath(std::string email);
std::map<std::string, std::string> initMimeTypes( void );
long long getCurrentTime();
std::string getFileContent(std::string filePath);
void closeAll(ServerContext ServerContext);
int setNonBlocking(int fd);
void initRoutes(ServerContext &serverContext);
std::map<std::string, std::string> parseSimpleJSON(const std::string& body);
//POST ROUTES
void routeRequestPOST(HttpRequest &req, HttpResponse &res, ServerContext serverContext);
//GET ROUTES
void routeRequestGET(HttpRequest &req, HttpResponse &res, ServerContext serverContext);
//DELETE ROUTES
void routeRequestDELETE(HttpRequest &req, HttpResponse &res, ServerContext serverContext);
//CGI ROUTES
void routeRequestCGI(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
//POST CONTROLLER
void handleLogin(HttpRequest &req, HttpResponse &res);
void handleCreateAccount(HttpRequest &req, HttpResponse &res);
void uploadFile(HttpRequest &req, HttpResponse &res);
//GET CONTROLLER
void handleGetFile(HttpRequest &req, HttpResponse &res);
void getFileNames(HttpRequest &req, HttpResponse &res);
void checkRootPassword(HttpRequest &req, HttpResponse &res);
//DELETE CONTROLLER
void delteFile(HttpRequest &req, HttpResponse &res);
//CGI CONTROLLER
void handleLs(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
void handleLoop(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
void executeSkript(HttpRequest &req, HttpResponse &res, ServerContext &serverContext, int clientFd);
//PAGES
void handleFileResponse(HttpResponse &res, const std::string &filePath, const std::string &contentType, int statusCode, const std::string &defaultMessage);
void handle400(HttpResponse &res);
void handle401(HttpResponse &res);
void handle403(HttpResponse &res);
void handle404(HttpResponse &res);
void handle405(HttpResponse &res);
void handle500(HttpResponse &res);
void handleHome(HttpResponse &res);
void handleIndexSstyle(HttpResponse &res);
void handleIndexJs(HttpResponse &res);
void handleIndexImgJkarras(HttpResponse &res);
void handleIndexImgAtoepper(HttpResponse &res);
void handleIndexImgRmathes(HttpResponse &res);
void handleIndexImgLogo(HttpResponse &res);
void handleRemoteStorageJs(HttpResponse &res);
void handleDashboard(HttpResponse &res);
void handleDashboardStyle(HttpResponse &res);
void handleDashboardJs(HttpResponse &res);
//LOGGER
void logger(std::string message, int code);
#endif
