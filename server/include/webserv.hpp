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
#include "../classes/header/ConficData.hpp"
#include <sstream>
#include <cerrno>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>

#define MAX_EVENTS 1024
#define MAX_CLIENTS 1024
#define PORT 8080
#define CHUNK_SIZE 1024
#define BUFFER_SIZE 1024

#define HTTP_BAD_REQUEST 400


enum RequestState {
	REQUEST_LINE,
	HEADERS,
	BODY,
	COMPLETE,
	ERROR
};

struct HttpRequest {
	std::string method, uri, version;
	std::map<std::string, std::string> headers;
	std::string body;
	RequestState state;
	int exitStatus;
	long long startTime;
	HttpRequest() : state(REQUEST_LINE) {}
};

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
};

//CONFIC
bool parseConfic(std::string path, ConficData *data);

//HTTPPARSER
void printHttpRequest(const HttpRequest& request);
void parseHttpRequest(HttpRequest &req, std::string &buffer);
//SERVER
void startServer(void);
//SERVER CONFIC
void startServerWithConfic(ConficData &data);
//SIG
void handle_sigint(int sig, siginfo_t *siginfo, void *context);

//HANDLE REQ
void handleRequest(int clientFd, ServerContext &ServerContext);
void handleErrorRequest(int clientFd, ServerContext &ServerContext);
//helper
std::string toString(int number);
std::string toString(long long number);
long long getCurrentTime();
#endif
