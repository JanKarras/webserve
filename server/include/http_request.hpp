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

struct HttpRequest {
	std::string method; /* perspektivisch  */
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