#include "../../include/webserv.hpp"

static const std::string validPathChars = "-_.~!$&'()*+,;=";
static const std::string validQueryChars = "-_.~!$'()*+,;:@/";

static void setRequestError(HttpRequest &req, int errType)
{
	req.buffer.clear();
	req.state = ERROR;
	req.exitStatus = errType;
}

// static bool isValidPathIdentifier(char c)
// {
// 	if (validPathChars.find(c) != std::string::npos || isalnum(c))
// 		return true;
// 	return false;
// }

// static bool isValidQueryIdentifier(char c)
// {
// 	if (validQueryChars.find(c) != std::string::npos || isalnum(c))
// 		return true;
// 	return false;
// }

// static char hexToAscii(const std::string& hex)
// {
// 	int value;
// 	std::stringstream ss;
// 	ss << std::hex << hex;
// 	ss >> value;
// 	return static_cast<char>(value);
// }

// static int resolvePath(HttpRequest &req, bool resolvePath)
// {
// 	if (!resolvePath)
// 		return SUCCESS;
// 	std::stack<std::string> dirs;  // Stack to hold directory components
// 	std::stringstream stream(req.path);
// 	std::string token;

// 	while (std::getline(stream, token, '/'))
// 	{
// 		if (token == "." || token.empty())
// 			continue;
// 		else if (token == "..")
// 		{
// 			if (dirs.empty())
// 			{
// 				setRequestError(req, HTTP_FORBIDDEN);
// 				return FAILURE;
// 			}
// 			else
// 				dirs.pop();
// 		}
// 		else
// 			dirs.push(token);
// 	}

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
					/* check method with strcmp */
					if (!req.buffer.compare(0, p, "GET"))
						req.method = GET;
					else if (!req.buffer.compare(0, p, "POST"))
						req.method = POST;
					else if (!req.buffer.compare(0, p, "DELETE"))
						req.method = DELETE;
					else
					{
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
				if (uriLength != std::string::npos)
				{
					req.uri = req.buffer.substr(p, uriLength);
					parseUri(req);
					/* check for cgi */
					p += uriLength;
					state = RL_VERSION;
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
						setRequestError(req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
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
	size_t keyPos, valPos;
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
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;
				break;
			case (HL_KEY):
				if (c == ':')
				{
					req.currentKey = req.buffer.substr(0, pos);
					req.buffer.erase(0, pos);
					pos = 0;
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
				{
					state = HL_VALUE;
				}
				else if (c == 'r')
					state = HL_END_OF_FIELD;
				else
				{
					setRequestError(req, HTTP_BAD_REQUEST);
					return FAILURE;					
				}
				break;
			case (HL_VALUE):
				if (c == ',' || c == ';')
				{
					req.headers[req.currentKey].push_back(req.buffer.substr(2, pos - 2));
					req.buffer.erase (0, pos);
					state = HL_COLON;
				}
				else if (c == '\r')
				{
					req.headers[req.currentKey].push_back(req.buffer.substr(2, pos - 2));
					req.buffer.erase (0, pos);
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
				break;
			case (HL_DOUBLE_QUOTES):
				if (c == '\"')
					state = HL_VALUE;
				if (c == '\\')
					state = HL_ESCAPE_CHAR;
				else if (c == '\r' || c == '\n')
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
					state = HL_KEY;
				else if (c == '\r')
					state = HL_DONE;
				else if (c > 33 && c < 126 && req.folding)
				{
					req.buffer.erase(0, pos - 2);
					pos = 2;
					req.folding = false;
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
				if (c != '\n')
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
					req.state = BODY;
				}
				else
					req.state = NO_BODY;
				req.parseState = 0;
				return SUCCESS;
			default:
				setRequestError(req, HTTP_BAD_REQUEST);
				return FAILURE;
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

/* to be corrected */
// static int parseHttpBody(HttpRequest &req)
// {
// 	req.body.append(req.buffer);
// 	req.buffer.clear();
	
// 	if (req.headers.count("Content-Length"))
// 	{
// 		char *endptr = NULL;
// 		unsigned long contentLength = strtoul(req.headers["Content-Length"].c_str(), &endptr, 10);
		
// 		// Check if conversion was successful
// 		if (*endptr != '\0') {
// 			setRequestError(req, HTTP_BAD_REQUEST);
// 			return FAILURE;
// 		}
		
// 		if (req.body.size() >= contentLength)
// 		{
// 			req.state = COMPLETE;
// 		}
// 	}
// 	return SUCCESS;
// }

/* static int parseQueryString(HttpRequest &req)
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
} */

/*  provisionally uri parser */
/* static int parseUri(HttpRequest &req)
{
	size_t pos = req.uri.find('?');
	if (pos != std::string::npos)
	{
		req.path = req.uri.substr(0, pos);
		req.queryString = req.uri.substr(pos + 1);
		parseQueryString (req);
	}
	else
	{
		req.path = req.uri;
		req.queryString = "";
	}
	if (req.path.rfind(".py") != std::string::npos || req.path.rfind(".sh") != std::string::npos)
	req.cgi = true;
	return SUCCESS;
} */

/* to be corrected */
// static int parseHttpHeaderLine(HttpRequest &req)
// {
// 	size_t p;
// 	while (!req.buffer.empty() && req.state != COMPLETE) {
// 		p = req.buffer.find("\r\n");
// 		if (p == std::string::npos)
// 			break;

// 		std::string line = req.buffer.substr(0, p);
// 		req.buffer.erase(0, p + 2);  // Remove line + CRLF

// 		if (line.empty()) {
// 			req.state = req.headers.count("Content-Length") ? BODY : COMPLETE;
// 			break;
// 		}

// 		size_t splitPos = line.find(":");
// 		if (splitPos != std::string::npos) {
// 			std::string key = line.substr(0, splitPos);
// 			std::string value = line.substr(splitPos + 2);
// 			req.headers[key].push_back(value);
// 		}
// 	}
// 	return 0;
// }