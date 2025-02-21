#include <iostream>
#include <string>
#include <map>

enum HttpMethod 
{ 
	GET,
	POST,
    DELETE
};

enum RequestState {
	REQUEST_LINE,
	HEADERS,
	BODY,
	COMPLETE,
	ERROR
};

enum RequestLineState{
	RL_START = 0,
	RL_METHOD,
	RL_URI,
	RL_VERSION,
	RL_DONE,
	RL_ERROR
};

enum HeaderLineState{
	HL_START = 0,
	HL_NAME,
	HL_VALUE,
	HL_DONE,
	HL_ERROR
};

enum BodyState{
	B_START = 0,
	B_DONE,
	B_ERROR,
};

struct HttpRequest {
	HttpMethod method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string buffer; // Stores the accumulated raw request data
	size_t content_length;
    size_t pos;
	unsigned int parseState;
	RequestState state;
	int exitStatus;
	long long startTime;
	HttpRequest() : state(REQUEST_LINE) {}
};