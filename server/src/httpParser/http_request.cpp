#include "../../include/webserv.hpp"

static const std::string validPathChars = "-_.~!$&'()*+,;=";
static const std::string validQueryChars = "-_.~!$'()*+,;:@/";
static const std::string validQuoteChars = "\t !#$%&'()*+,-./:;<=>?@[]_{}~";
static const std::string validValueChars = "\"!#$%&'*+-.^_`|~";
static int distributeRequest(ConfigData &config, HttpRequest &req);

static std::string toLowerCase(const std::string& str)
{
	std::string lower = "";
	for (size_t i = 0; i < str.length(); i++)
		lower.append(1, tolower(str[i]));
	return lower;
}

static void setRequestError(ConfigData &configData, HttpRequest &req, int errType)
{
	req.buffer.clear();
	req.state = ERROR;
	req.exitStatus = errType;
	if (req.srv == NULL)
		distributeRequest (configData, req);
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

static int resolvePath(ConfigData &configData, HttpRequest &req, bool resolvePath)
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
				setRequestError(configData, req, HTTP_FORBIDDEN);
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
			value = req.queryString.substr(startPos);
		else
			value = req.queryString.substr(startPos, endPos - startPos);

		req.query[key] = value;

		startPos = (endPos == std::string::npos) ? req.queryString.length() : endPos + 1;
	}
	return SUCCESS;
}

static int distributeRequest(ConfigData &config, HttpRequest &req)
{
	if (req.headers.find("host") != req.headers.end())
	{
		std::string serverName = req.headers["host"];
		for (size_t i = 0; i < config.servers.size(); i++)
		{
			if (config.servers[i].server_name == serverName)
			{
				req.srv = &config.servers[i];
				config.servers[i].serverContex.requests.insert(std::pair<int, HttpRequest *>(req.clientFd, &req));
				return SUCCESS;
			}
		}
	}
	req.srv = &config.servers[0];
	config.servers[0].serverContex.requests.insert(std::pair<int, HttpRequest *>(req.clientFd, &req));
	return SUCCESS;
}

static int parseUri(ConfigData &configData, HttpRequest &req)
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
			setRequestError(configData, req, HTTP_BAD_REQUEST);
			return FAILURE;
			break;
		default:
			break;
		}
	}
	if ((state == URI_PATH_SEGMENT || state == URI_AFTER_SLASH) && resolvePath(configData, req, resolve) == SUCCESS)
	{
		req.path = req.uri;
		return SUCCESS;
	}
	if ((state == URI_EQUAL || state == URI_VALUE) && resolvePath(configData, req, resolve) == SUCCESS)
	{
		req.queryString = req.uri.substr(queryPos + 1);
		extractQuery(req);
		return SUCCESS;
	}

	setRequestError(configData, req, HTTP_BAD_REQUEST);
	return FAILURE;
}

static int parseHttpRequestLine(ConfigData &configData, HttpRequest &req)
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
					setRequestError(configData, req, HTTP_BAD_REQUEST);
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
					else if (!req.buffer.compare(0, p, "HEAD"))
						req.method = HEAD;
					else if (!req.buffer.compare(0, p, "PUT"))
						req.method = PUT;
					else if (!req.buffer.compare(0, p, "CONNECT"))
						req.method = CONNECT;
					else if (!req.buffer.compare(0, p, "OPTIONS"))
						req.method = OPTIONS;
					else if (!req.buffer.compare(0, p, "TRACE"))
							req.method = TRACE;
					else if (!req.buffer.compare(0, p, "PATCH"))
						req.method = PATCH;
					else
					{
						req.method = INVALID;
						setRequestError(configData, req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
					state = RL_URI;
				}
				else if (c < 'A' || c > 'Z' || p > 6)
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case RL_URI:
				uriLength = req.buffer.find(' ', p) - p;

				if (uriLength != std::string::npos - p)
				{
					req.uri = req.buffer.substr(p, uriLength);

					if (parseUri(configData, req) == FAILURE)
						return FAILURE;
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
						setRequestError(configData, req, HTTP_BAD_REQUEST);
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

static int parseHttpHeaderLine(ConfigData &configData, HttpRequest &req)
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
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_KEY):
				if (c == ':')
				{
					req.currentKey = toLowerCase(req.buffer.substr(0, pos));
					req.headers[req.currentKey];
					state = HL_COLON;
				}
				else if (c != '-' && !isalnum(c))
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_COLON):
				if (c == ' ' || c == '\t')
					state = HL_SPACE_AFTER_COLON;
				else if (c == '\r')
				{
					req.headers[req.currentKey] = "";
					state = HL_END_OF_FIELD;
				}
				else if (c >= 33 && c <= 126)
				{
					req.valuePos = pos;
					if (c == '\"')
						state = HL_DOUBLE_QUOTES;
					else
						state = HL_VALUE;
				}
				else
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_SPACE_AFTER_COLON):
				if (c == ' ' || c == '\t')
					state = HL_SPACE_AFTER_COLON;
				else if (c >= 33 && c <= 126)
				{
					req.valuePos = pos;
					if (c == '\"')
						state = HL_DOUBLE_QUOTES;
					else
						state = HL_VALUE;
				}
				else
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_VALUE):
				if (c == '\r')
				{
					req.headers[req.currentKey] = req.buffer.substr(req.valuePos, pos - req.valuePos);
					state = HL_END_OF_FIELD;
				}
				else if (c == '\"')
					state = HL_DOUBLE_QUOTES;
				else if (c < 32 || ( c > 126 && c != '\t'))
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
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
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_ESCAPE_CHAR):
				if (c == '\"' || c == '\\')
					state = HL_DOUBLE_QUOTES;
				else
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_END_OF_FIELD):
				if (c != '\n')
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
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
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (HL_DONE):
				if (c != '\n' || req.headers.empty() || req.headers.find("host") == req.headers.end()) //host oder Host???
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				req.buffer.erase(0, pos + 1);
				req.pos = 0;
				distributeRequest(configData, req);
				std::map<std::string, std::string>::iterator it = req.headers.find("transfer-encoding");
				if (it != req.headers.end() && it->second == "chunked")
				{
					if (req.headers.find("content-length") != req.headers.end())
					{
						setRequestError(configData, req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
					else
						req.state = BODY_CHUNKED;
				}
				else if (req.headers.find("content-length") != req.headers.end())
				{
					long tmp = atol(req.headers["content-length"].c_str());
					req.content_length = tmp;
					if (tmp < 0)
					{
						setRequestError(configData, req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
					else if (req.content_length > req.srv->client_max_body_size)
					{
						setRequestError(configData, req, HTTP_ENTITY_TOO_LARGE);
						return FAILURE;
					}
					req.state = BODY;
				}
				else
					req.state = NO_BODY;
				req.parseState = 0;
				return SUCCESS;
		}
		buffer_length = req.buffer.length();
	}
	req.pos = pos;
	req.parseState = static_cast<int>(state);
	return SUCCESS;
}

static int parseHttpNoBody(ConfigData &configData, HttpRequest &req)
{
	if (!req.buffer.empty())
	{
		setRequestError(configData, req, HTTP_BAD_REQUEST);
		return FAILURE;
	}
	req.state = COMPLETE;
	return SUCCESS;
}

static int parseHttpBody(ConfigData &configData, HttpRequest &req)
{
	size_t bufferLen = req.buffer.length();

	size_t remainingBytes = req.content_length - req.body.size();
	if (bufferLen > remainingBytes)
	{
		setRequestError(configData, req, HTTP_ENTITY_TOO_LARGE);
		return FAILURE;
	}

	req.body.append(req.buffer, 0, bufferLen);
	req.buffer.erase(0, bufferLen);
	req.pos = 0;

	if (req.body.size() == req.content_length)
	req.state = COMPLETE;
	return SUCCESS;
}

static int parseHttpBodyChunked(ConfigData &configData, HttpRequest &req)
{
	size_t pos = req.pos;
	BodyState state = static_cast<BodyState>(req.parseState);
	size_t buffer_length = req.buffer.length();
	for (; pos < buffer_length; pos++)
	{
		u_char c = req.buffer[pos];

		switch (state)
		{
			case (B_CHUNK_SIZE_START):
				if (pos != 0)
				{
					req.buffer.erase(0, pos);
					pos = 0;
					c = req.buffer[pos];
					buffer_length = req.buffer.length();
				}
				if (!isxdigit(c))
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				state = B_CHUNK_SIZE;
				break;
			case (B_CHUNK_SIZE):
				if (c == '\r')
				{
					std::string hexSize = req.buffer.substr(0, pos);
					char *endptr;
					long tmp = strtol(hexSize.c_str(), &endptr, 16);
					req.chunkSize = tmp;
					if (*endptr != '\0' || tmp < 0)
					{
						setRequestError(configData, req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
					state = B_CHUNK_SIZE_CRLF;
				}
				else if (!isxdigit(c))
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				break;
			case (B_CHUNK_SIZE_CRLF):
				if (c != '\n')
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				if (req.chunkSize == 0)
					state = B_CHUNK_TRAILER_START;
				else
					state = B_CHUNK_DATA_START;
				break;
			case (B_CHUNK_DATA_START):
				req.buffer.erase(0, pos);
				pos = 0;
				if (c == '\r')
					state = B_CHUNK_DATA_CRLF;
				else
					state = B_CHUNK_DATA;
				break;
			case (B_CHUNK_DATA):
				if (pos > req.chunkSize)
				{
					setRequestError(configData, req, HTTP_ENTITY_TOO_LARGE);
					return FAILURE;
				}
				else if (c == '\r')
					state = B_CHUNK_DATA_CRLF;
				break;
			case (B_CHUNK_DATA_CRLF):
				if (c == '\n' )
				{
					if (pos - 1 == req.chunkSize)
					{
						if (req.body.size() + req.chunkSize > req.srv->client_max_body_size)
						{
							setRequestError(configData, req, HTTP_ENTITY_TOO_LARGE);
							return FAILURE;
						}
						req.body.append(req.buffer.substr(0, req.chunkSize));
						state = B_CHUNK_SIZE_START;
					}
					else
					{
						Logger::error("HTTP_BAD_REQUEST");
						setRequestError(configData, req, HTTP_BAD_REQUEST);
						return FAILURE;
					}
				}
				else
					state = B_CHUNK_DATA;
				break;
			case (B_CHUNK_TRAILER_START):
				req.buffer.erase(0, pos);
				pos = 0;
				if (c == '\r')
					state = B_FINAL_CRLF;
				else if (!isalnum(c))
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				else
					state = B_CHUNK_TRAILER;
				break;
			case (B_CHUNK_TRAILER):
				if (c == '\r')
					state = B_CHUNK_TRAILER_CRLF;
				break;
			case (B_CHUNK_TRAILER_CRLF):
				if (c != '\n')
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				else
					state = B_CHUNK_TRAILER_START;
				break;
			case (B_FINAL_CRLF):
				req.buffer.erase(0, pos);
				if (c != '\n' && req.buffer.size() != 0)
				{
					setRequestError(configData, req, HTTP_BAD_REQUEST);
					return FAILURE;
				}
				else
				{
					req.content_length = req.body.size();
					req.state = COMPLETE;
				}
				return SUCCESS;
			break;
		}
		buffer_length = req.buffer.length();
	}
	req.pos = pos;
	req.parseState = static_cast<int>(state);
	return SUCCESS;
}

void parseHttpRequest(ConfigData &configData, int client_fd, std::string &data)
{
	HttpRequest &req = configData.requests[client_fd];
	req.startTime = getCurrentTime();
	req.buffer.append(data);
	//Logger::debug("Data appended to buffer for client %i", client_fd);

	if (req.state == REQUEST_LINE) {
		if (parseHttpRequestLine(configData, req) == FAILURE) return;
		//Logger::debug("Request line parsed successfully for client %i", client_fd);
	}

	if (req.state == HEADERS) {
		if (parseHttpHeaderLine(configData, req) == FAILURE) return;
		//Logger::debug("Headers parsed successfully for client %i", client_fd);
	}

	if (req.state == NO_BODY) {
		if (parseHttpNoBody(configData, req) == FAILURE) return;
		//Logger::debug("No-body request handled successfully for client %i", client_fd);
	}

	if (req.state == BODY_CHUNKED) {
		if (parseHttpBodyChunked(configData, req) == FAILURE) return;
		//Logger::debug("Chunked body parsed successfully for client %i", client_fd);
	}

	if (req.state == BODY) {
		if (parseHttpBody(configData, req) == FAILURE) return;
		//Logger::debug("Body parsed successfully for client %i", client_fd);
	}
}
