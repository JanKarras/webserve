#include <iostream>
#include <string>
#include <map>
#include <stack>

enum HttpMethod
{
	GET,
	POST,
    DELETE,
	INVALID
};

enum UriParseState{
	URI_START = 0,
	URI_AFTER_SLASH,
	URI_PATH_SEGMENT,
	URI_PERCENTAGE,
	URI_PATH_EXPANDER,
	URI_QUERY,
	URI_KEY,
	URI_EQUAL,
	URI_VALUE,
	URI_PERCENTAGE_KEY,
	URI_PERCENTAGE_VALUE,
	URI_ERROR,
};

enum RequestState {
	REQUEST_LINE = 0,
	HEADERS,
	NO_BODY,
	BODY,
	BODY_CHUNKED,
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
	HL_KEY,
	HL_COLON,
	HL_SPACE_AFTER_COLON,
	HL_VALUE,
	HL_COMMA,
	HL_END_OF_FIELD,
	HL_FOLDING,
	HL_DOUBLE_QUOTES,
	HL_ESCAPE_CHAR,
	HL_DONE
	// HL_ERROR
};

enum BodyState{
	// B_START = 0,
	B_CHUNK_SIZE = 0,
	B_CHUNK_DATA,
	B_CHUNK_TRAILER
	// B_DONE,
	// B_ERROR,
};



struct HttpRequest {
	HttpMethod method;
	std::string uri;
	std::string version;
	std::string path;
	std::string queryString;
	std::map<std::string, std::vector<std::string> > headers;
	std::map<std::string, std::string> query;
	std::string		body;
	std::string		buffer; // Stores the accumulated raw request data
	size_t			content_length;
	bool			cgi;
    size_t			pos;
	bool			folding;
	std::string		currentKey;
	size_t			valuePos;
	size_t			chunkSize;
	server			*server;

	unsigned int parseState;
	RequestState state;
	int exitStatus;
	long long startTime;
	HttpRequest() : cgi(false), pos (0), folding (false), currentKey(""), parseState(0), state(REQUEST_LINE) {}
};

std::ostream& operator<<(std::ostream& os, const HttpMethod& method);
std::ostream& operator<<(std::ostream& os, const RequestState& state);
std::ostream& operator<<(std::ostream& os, const RequestLineState& state);
