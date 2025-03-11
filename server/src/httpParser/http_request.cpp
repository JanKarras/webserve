#include "../../include/webserv.hpp"

static const std::string validPathChars = "-_.~!$&'()*+,;=";
static const std::string validQueryChars = "-_.~!$'()*+,;:@/";
static const std::string validQuoteChars = "\t !#$%&'()*+,-./:;<=>?@[]_{}~";
static const std::string validValueChars = "\"!#$%&'*+-.^_`|~";

static void setRequestError(HttpRequest &req, int errType)
{
	req.buffer.clear();
	req.state = ERROR;
	req.exitStatus = errType;
}

static bool isValidPathIdentifier(char c)
{
	if (validPathChars.find(c) != std::string::npos || isalnum(c))
		return true;
	return false;
}

static bool isValidQueryIdentifier(char c)
{
	if (validQueryChars.find(c) != std::string::npos || isalnum(c))
		return true;
	return false;
}

static char hexToAscii(const std::string& hex)
{
	int value;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> value;
	return static_cast<char>(value);
}

static int resolvePath(HttpRequest &req, bool resolvePath)
{
	if (!resolvePath)
		return SUCCESS;
	std::stack<std::string> dirs;  // Stack to hold directory components
	std::stringstream stream(req.path);
	std::string token;

	while (std::getline(stream, token, '/'))
	{
		if (token == "." || token.empty())
			continue;
		else if (token == "..")
		{
			if (dirs.empty())
			{
				setRequestError(req, HTTP_FORBIDDEN);
				return FAILURE;
			}
			else
				dirs.pop();
		}
		else
			dirs.push(token);
	}

	// Reconstruct the resolved path
	std::string resolvedPath = "/";
	std::stack<std::string> tempStack;
	while (!dirs.empty())
	{
		tempStack.push(dirs.top());
		dirs.pop();
	}
	// Concatenate each directory to form the final resolved path
	while (!tempStack.empty())
	{
		resolvedPath += tempStack.top() + "/";
		tempStack.pop();
	}
	// If the path isn't just the root, remove the last trailing slash
	if (resolvedPath.length() > 1)
		resolvedPath.erase(resolvedPath.length() - 1);
	req.path = resolvedPath;
	return SUCCESS;
}

static int extractQuery(HttpRequest &req)
{
	size_t startPos = 0;
	size_t endPos;
	size_t queryLen = req.queryString.length();
	std::string key, value;

	while (startPos < queryLen)
	{
		endPos = req.queryString.find('=', startPos);
		if (endPos == std::string::npos)
			break;
		key = req.queryString.substr(startPos, endPos - startPos);
		startPos = endPos + 1;
		endPos = req.queryString.find('&', startPos);

		if (endPos == std::string::npos)
			value = req.queryString.substr(startPos); // Last key=value pair
		else
			value = req.queryString.substr(startPos, endPos - startPos);

		req.query[key] = value;

		startPos = (endPos == std::string::npos) ? req.queryString.length() : endPos + 1;
	}
	return SUCCESS;
}

/* 
static bool	isCgi(std::string &path, std::vector<std::string>cgiExtension)
{
	size_t pathLen = path.length();
	size_t extensionLen;
	std::vector<std::string>::iterator it = cgiExtension.begin();
	
	while (it != cgiExtension.end())
	{
		extensionLen = 
		if ( extension in string )
		return true;
	}
	return false;
}
*/

static int parseUri(HttpRequest &req)
{
	size_t uriLen = req.uri.length();
	size_t queryPos;
	bool resolve = false;
	UriParseState state = URI_START;

	for (size_t pos = 0; pos < uriLen; pos++)
	{
		char c = req.uri[pos];
		switch (state)
		{
		case URI_START:
			if (c == '/')
				state = URI_AFTER_SLASH;
			else
				state = URI_ERROR;
			break;
		case URI_AFTER_SLASH:
			if (c == '%')
				state = URI_PERCENTAGE;
			else if (c == '.')
				state = URI_PATH_EXPANDER;
			else if (c == '?')
				{
					req.path = req.uri.substr(0, pos);
					queryPos = pos;
					state = URI_QUERY;
				}
			else if (isValidPathIdentifier(c))
				state = URI_PATH_SEGMENT;
			else
				state = URI_ERROR;
			break;
		case URI_PATH_SEGMENT:
			if (c == '%')
				state = URI_PERCENTAGE;
			else if (c == '/')
				state = URI_AFTER_SLASH;
			else if (c == '?')
			{
				req.path = req.uri.substr(0, pos);
				queryPos = pos;
				state = URI_QUERY;
			}
			else if (isValidPathIdentifier(c))
				state = URI_PATH_SEGMENT;
			else
				state = URI_ERROR;
			break;
		case URI_PERCENTAGE:
			if (pos + 1 < uriLen && isxdigit(c) && isxdigit(req.uri[pos + 1]))
			{
				char ascii = hexToAscii(req.uri.substr(pos, 2));
				req.uri.replace(pos - 1, 3, 1, ascii);
				state =  URI_PATH_SEGMENT;
			}
			else
				state = URI_ERROR;
			break;
		case URI_PATH_EXPANDER:
			if (c == '/')
			{
				resolve = true;
				state = URI_AFTER_SLASH;
			}
			else if (pos + 1 < uriLen && c == '.' && req.uri[pos + 1] == '/')
			{
				resolve = true;
				pos++;
				state = URI_AFTER_SLASH;
			}
			else
			{
				pos--;
				state = URI_PATH_SEGMENT;
			}
			break;
		case URI_QUERY:
			if (isValidQueryIdentifier(c))
				state = URI_KEY;
			else 
				state = URI_ERROR;
			break;
		case URI_KEY:
			if (c == '=')
				state = URI_EQUAL;
			else if (c == '%')
				state = URI_PERCENTAGE_KEY;
			else if (isValidQueryIdentifier(c))
				state = URI_KEY;
			else
				state = URI_ERROR;
			break;
		case URI_EQUAL:
			if (c == '&')
				state = URI_QUERY;
			else if (isValidQueryIdentifier(c))
				state = URI_VALUE;
			else
				state = URI_ERROR;
			break;
		case URI_VALUE:
			if (c == '&')
				state = URI_QUERY;
			else if (c == '%')
					state = URI_PERCENTAGE_VALUE;
			else if (isValidQueryIdentifier(c))
				state = URI_VALUE;
			else
				state = URI_ERROR; 
			break;
		case URI_PERCENTAGE_KEY:
			if (pos + 1 < uriLen && isxdigit(c) && isxdigit(req.uri[pos + 1]))
			{
				char ascii = hexToAscii(req.uri.substr(pos, 2));
				req.uri.replace(pos - 1, 3, 1, ascii);
				state =  URI_PATH_SEGMENT;
			}
			else
				state = URI_KEY;
			break;
		case URI_PERCENTAGE_VALUE:
			if (pos + 1 < uriLen && isxdigit(c) && isxdigit(req.uri[pos + 1]))
			{
				char ascii = hexToAscii(req.uri.substr(pos, 2));
				req.uri.replace(pos - 1, 3, 1, ascii);
				state =  URI_VALUE;
			}
			else
				state = URI_ERROR;
			break;
		case URI_ERROR:
			setRequestError(req, HTTP_BAD_REQUEST);
			return FAILURE;
			break;
		default:
			break;
		}
	}

	if ((state == URI_PATH_SEGMENT || state == URI_AFTER_SLASH) && resolvePath(req, resolve) == SUCCESS)
	{
		req.path = req.uri;
		return SUCCESS;
	}
	if ((state == URI_EQUAL || state == URI_VALUE) && resolvePath(req, resolve) == SUCCESS)
	{
		req.queryString = req.uri.substr(queryPos + 1);
		extractQuery(req);
		return SUCCESS;
	}

	setRequestError(req, HTTP_BAD_REQUEST);
	return FAILURE;
}

static int parseHttpRequestLine(HttpRequest &req)
{
	size_t p = req.pos;
	size_t uriLength;
	RequestLineState state = static_cast<RequestLineState>(req.parseState);
	size_t buffer_length = req.buffer.length();

	for (; p < buffer_length; p++)
	{
		u_char c = req.buffer[p];

		switch (state)
		{
			case RL_START:
				if (c == CR || c == LF)
					break;
				if (c < 'A' || c > 'Z')
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				state = RL_METHOD;
				break;
			case RL_METHOD:
				if (c == ' ')
				{
					if (!req.buffer.compare(0, p, "GET"))
						req.method = GET;
					else if (!req.buffer.compare(0, p, "POST"))
						req.method = POST;
					else if (!req.buffer.compare(0, p, "DELETE"))
						req.method = DELETE;
					else
					{
						req.method = INVALID;
						setRequestError(req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
					state = RL_URI;
				}
				else if (c < 'A' || c > 'Z' || p > 5)
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case RL_URI:
				uriLength = req.buffer.find(' ', p) - p;

				if (uriLength != std::string::npos - p)
				{
					req.uri = req.buffer.substr(p, uriLength);

					if (parseUri(req) == FAILURE)
						return FAILURE;
					/* if (isCgi(req.path, server.cgi))
						req.cgi = true; */
					p += uriLength;
					state = RL_VERSION;
				}
				else
				{
					req.parseState = state;
					req.pos = p;	
					return SUCCESS;
				}
				break;
			case RL_VERSION:
				if (buffer_length - p >= 10)
				{
					if (!req.buffer.compare(p, 10, "HTTP/1.1\r\n"))
					{
						req.version = "HTTP/1.1";
						p += 10;
						state = RL_DONE;
					}
					else
					{
						state = RL_ERROR;
						setRequestError(req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
				}
				else
				{
					req.parseState = state;
					req.pos = p;	
					return SUCCESS;
				}
				break;
			default:
				break;
			}
		if (state == RL_DONE)
			break;
	}
	req.pos = p;
	req.parseState = static_cast<int>(state);
	if (state == RL_DONE)
	{
		req.parseState = 0;
		req.state = HEADERS;
		req.buffer.erase(0, p);
		req.pos = 0;
		return SUCCESS;
	}
	req.state = REQUEST_LINE;
	return SUCCESS;
}

static int parseHttpHeaderLine(HttpRequest &req)
{
	size_t pos = req.pos;
	HeaderLineState state = static_cast<HeaderLineState>(req.parseState);
	size_t buffer_length = req.buffer.length();

	for (; pos < buffer_length; pos++)
	{
		u_char c = req.buffer[pos];

		switch (state)
		{
			case (HL_START):
				if (isalnum(c))
					state = HL_KEY;
				else if (c == '\r')
					state = HL_DONE;
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_KEY):
				if (c == ':')
				{
					req.currentKey = req.buffer.substr(0, pos);
					req.headers[req.currentKey];
					state = HL_COLON;
				}
				else if (c != '-' && !isalnum(c))
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}	
				break;
			case (HL_COLON):
				if (c == ' ' || c == '\t')
					state = HL_SPACE_AFTER_COLON;
				else if (c == '\r')
				{
					req.headers[req.currentKey].push_back("");
					state = HL_END_OF_FIELD;
				}
				else if (c == ',' || c == ';')
				{
					req.headers[req.currentKey].push_back("");
					state = HL_COMMA;
				}
				else if (validValueChars.find(c) != std::string::npos || isalnum(c))
				{
					req.valuePos = pos;
					if (c == '\"')
						state = HL_DOUBLE_QUOTES;
					else
						state = HL_VALUE;
				}
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;					
				}
				break;
			case (HL_SPACE_AFTER_COLON):
				if (c == ' ' || c == '\t')
					state = HL_SPACE_AFTER_COLON;
				else if (c == ',' || c == ';')
				{
					req.headers[req.currentKey].push_back("");
					state = HL_COMMA;
				}
				else if (validValueChars.find(c) != std::string::npos || isalnum(c))
				{
					req.valuePos = pos;
					if (c == '\"')
						state = HL_DOUBLE_QUOTES;
					else	
						state = HL_VALUE;
				}
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;					
				}
				break;
			case (HL_VALUE):
				if (c == ',' || c == ';')
				{
					req.headers[req.currentKey].push_back(req.buffer.substr(req.valuePos, pos - req.valuePos));
					state = HL_COLON;
				}
				else if (c == '\r')
				{
					req.headers[req.currentKey].push_back(req.buffer.substr(req.valuePos, pos - req.valuePos));
					state = HL_END_OF_FIELD;
				}
				else if (c == '\"')
					state = HL_DOUBLE_QUOTES;
				else if (c < 33 || c > 126)
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;					
				}
				break;
			case (HL_COMMA):
				if (c == ' ' || c == '\t')
				{
					state = HL_SPACE_AFTER_COLON;
				}
				else if (c == '\r')
					state = HL_END_OF_FIELD;
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;					
				}
				break;
			case (HL_DOUBLE_QUOTES):
				if (c == '\"')
					state = HL_VALUE;
				else if (c == '\\')
					state = HL_ESCAPE_CHAR;
				else if (isalnum(c) || validQuoteChars.find(c) != std::string::npos)
					break;
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;						
				}
				break;
			case (HL_ESCAPE_CHAR):
				if (c == '\"' || c == '\\') 
				{
					// req.buffer.replace(pos - 1, 2, "\"");
					// pos--;
					state = HL_DOUBLE_QUOTES;
				}
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_END_OF_FIELD):
				if (c != '\n')
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				state = HL_FOLDING;
				break;
			case (HL_FOLDING):
				if (isalnum(c) && !req.folding)
				{
					req.buffer.erase(0, pos);
					pos = 0;
					state = HL_KEY;
				}
				else if (c == '\r')
					state = HL_DONE;
				else if (c > 33 && c < 126 && req.folding)
				{
					req.valuePos = pos;
					req.folding = false;
					if (c == '\"')
						state = HL_DOUBLE_QUOTES;
					else
						state = HL_VALUE;
				}
				else if (c == ' ' || c == '\t')
					req.folding = true;
				else 
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;					
				}
				break;
			case (HL_DONE):
				if (c != '\n' || req.headers.empty())
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				req.buffer.erase(0, pos + 1);
				req.pos = 0;
				std::map<std::string, std::vector<std::string>>::iterator it = req.headers.find("Transfer-Encoding");
				if (it != req.headers.end() && it->second[0] == "chunked")
					req.state = BODY_CHUNKED;
				else if (req.headers.find("Content-Length") != req.headers.end())
				{
					req.content_length = atol(req.headers["Content-Length"][0].c_str());
					if (req.content_length < 0)
					{
						setRequestError(req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
					// else if (req.content_length < CONTENT_MAX)
					// {
					// 	setRequestError(req, HTTP_ENTITY_TOO_LARGE);
					// 	return FAILURE;
					// }
					req.state = BODY;
				}
				else
					req.state = NO_BODY;
				req.parseState = 0;
				return SUCCESS;
			// default:
			// 	setRequestError(req, HTTP_BAD_REQUEST);
			// 	return FAILURE;
		}
		buffer_length = req.buffer.length();
	}
	req.pos = pos;
	req.parseState = static_cast<int>(state);
	return SUCCESS;
}

static int parseHttpNoBody(HttpRequest &req)
{
	if (!req.buffer.empty())
	{
		setRequestError(req, HTTP_BAD_REQUEST);
		return FAILURE;
	}
	req.state = COMPLETE;
	return SUCCESS;
}

static int parseHttpBody(HttpRequest &req)
{
	size_t pos = req.pos;
	size_t bufferLen = req.buffer.length();

	size_t remainingBytes = req.content_length - req.body.size();
	if (bufferLen > remainingBytes)
	{
		setRequestError(req, HTTP_BAD_REQUEST);
		return FAILURE;
	}

	req.body.append(req.buffer, 0, bufferLen);
	req.buffer.erase(0, bufferLen);
	req.pos = 0;

	if (req.body.size() == req.content_length)
		req.state = COMPLETE;
		return SUCCESS;
}

static int parseHttpBodyChunked(HttpRequest &req)
{
	BodyState state = static_cast<BodyState>(req.parseState);
	while (!req.buffer.empty())
	{
		switch (state)
		{
			case B_CHUNK_SIZE:
			{
				size_t pos = req.buffer.find("\r\n");
				if (pos == std::string::npos) // Incomplete chunk size
					return SUCCESS;

				// Convert hex size to integer
				std::string hexSize = req.buffer.substr(0, pos);
				char *endptr;
				long chunkSize = strtol(hexSize.c_str(), &endptr, 16);
				if (*endptr != '\0' || chunkSize < 0) // Invalid size
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				/*
				if (req.body.size() + chunkSize > req.content_length)
				{
					setRequestError(req, HTTP_TOO_LARGE);
					return FAILURE;
				}
				*/
				req.chunkSize = chunkSize;
				req.buffer.erase(0, pos + 2); // Remove size line
				state = (chunkSize == 0) ? B_CHUNK_TRAILER : B_CHUNK_DATA;
				break;
			}

			case B_CHUNK_DATA:
			{
				if (req.buffer.length() < req.chunkSize + 2) // Wait for full chunk
					return SUCCESS;

				req.body.append(req.buffer, 0, req.chunkSize);
				req.buffer.erase(0, req.chunkSize + 2); // Remove chunk + \r\n
				state = B_CHUNK_SIZE;
				break;
			}

			case B_CHUNK_TRAILER:
			{
				if (req.buffer.length() < 2) // Trailer must be \r\n
					return SUCCESS;

				if (req.buffer.substr(0, 2) != "\r\n") // Invalid trailer
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				}

				req.buffer.erase(0, 2); // Remove final \r\n
				req.state = COMPLETE;
				return SUCCESS;
			}
		}
	}
	return SUCCESS;
}

void parseHttpRequest(HttpRequest &req, std::string &data)
{
	req.buffer.append(data);
	
    if (req.state == REQUEST_LINE)
		if (parseHttpRequestLine(req) == FAILURE) return;
	
    if (req.state == HEADERS)
		if (parseHttpHeaderLine(req) == FAILURE) return;
	
	if (req.state == NO_BODY)
		if (parseHttpNoBody(req) == FAILURE) return;
	
	if (req.state == BODY_CHUNKED)
		if (parseHttpBodyChunked(req) == FAILURE) return;
	
    if (req.state == BODY)
		if (parseHttpBody(req) == FAILURE) return;
}

// void printRequest(const HttpRequest &req)
// {
// 	std::cout << "Method: " << req.method << "\n";
// 	std::cout << "URI: " << req.uri << "\n";
// 	std::cout << "Path: " << req.path << "\n";
// 	std::cout << "HTTP Version: " << req.version << "\n";
// 	std::cout << "Headers: " << "\n";
// 	for (const auto &header : req.headers)
// 	{
// 		if (!header.second.empty())
// 			std::cout << "  " << header.first << ": " << header.second[0] << "\n";
// 	}
// 	std::cout << "Body: " << req.body << "\n";
// 	std::cout << "State: " << req.state << "\n";
// 	std::cout << "---------------------------\n";
// }

// void testRequest(const std::string &rawRequest)
// {
//     HttpRequest req;
//     size_t chunkSize = 100; // Simulate receiving data in chunks
//     for (size_t i = 0; i < rawRequest.size(); i += chunkSize)
//     {
//         std::string chunk = rawRequest.substr(i, chunkSize);
//         parseHttpRequest(req, chunk);
//     }
//     printRequest(req);
// }

// int main()
// {
//     std::vector<std::string> testCases = {
//         // Valid GET request
//         "GET /index.html HTTP/1.1\r\nHost: example.com\r\nUser-Agent: TestClient\r\n\r\n",
        
//         // Valid POST request with body
//         "POST /submit HTTP/1.1\r\nHost: example.com\r\nContent-Length: 13\r\n\r\nHello, world!",
        
//         // Invalid request: missing HTTP version
//         "GET /index.html\r\nHost: example.com\r\n\r\n",
        
//         // Invalid request: unknown method
//         "FOO /bar HTTP/1.1\r\nHost: example.com\r\n\r\n",
        
//         // Valid request with header continuation (folded header)
//         "GET /index HTTP/1.1\r\nHost: example.com\r\nX-Header: value\r\n  continued-value\r\n\r\n",
        
//         // Invalid: No headers, should be rejected in HTTP/1.1
//         "GET / HTTP/1.1\r\n\r\n",

// 		// Invalid request: malformed header
//         "GET /home HTTP/1.1\r\nHost example.com\r\n\r\n",
        
//         // Valid request with multiple header values
//         "GET / HTTP/1.1\r\nHost: example.com\r\nAccept: text/html, application/json\r\n\r\n",
        
//         // Valid request with header key duplicates
//         "GET / HTTP/1.1\r\nHost: example.com\r\nAccept: text/html\r\nAccept: application/json\r\n\r\n",

// 		// Invalid request with empty header values
//         "GET / HTTP/1.1\r\nHost: example.com\r\nAccept: , , application/json\r\nrandom-key-with-space-after-colon: \r\n\r\n",

// 		// Valid request with empty header values
// 		"GET / HTTP/1.1\r\nHost: example.com\r\nAccept:	 ,		,  	q,\r\nTest:\r\n\r\n",

//         // Valid chunked transfer encoding request
//         "POST /upload HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n",
        
//         // // Invalid chunked body with incorrect size
//         "POST /upload HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\nHello\r\n0\r\n\r\n"
    
//     };

//     for (const auto &testCase : testCases)
//     {
//         testRequest(testCase);
//     }

//     return 0;
// }
