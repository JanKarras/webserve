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
#include <fstream>
#include <cerrno>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include "http_request.hpp"
#include <pthread.h>


#define SUCCESS 0
#define FAILURE 1

#define MAX_EVENTS 1024
#define MAX_CLIENTS 1024
#define PORT 8080
#define CHUNK_SIZE 1024
#define BUFFER_SIZE 1024

#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_N_ALLOWED 405
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
bool initSignal(void);
//HANDLE REQ
void handleRequest(int clientFd, ServerContext &ServerContext);
void handleErrorRequest(int clientFd, ServerContext &ServerContext);
//helper
std::string toStringInt(int number);
std::string toString(long long number);
long long getCurrentTime();
std::string getFileContent(std::string filePath);
#endif
