#include "../../include/webserv.hpp"

static const std::string validPathChars = "-_.~!$&'()*+,;=";
static const std::string validQueryChars = "-_.~!$'()*+,;:@/?";

static void setRequestError(HttpRequest &req, int errType)
{
	req.buffer.clear();
	req.state = ERROR;
	req.exitStatus = errType;
}

static void expandPath(HttpRequest &req)
{
	
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

/* final URI parser */
static int parseUri(HttpRequest &req)
{
	size_t uriLen = req.uri.length();
	size_t queryPos;
	bool expandPath = false;
	UriParseState state = URI_START;

	for (size_t pos = 0; pos < uriLen; pos++)
	{
		char c = req.uri[pos];
		switch (state)
		{
		case URI_START:
			if (c == '/')
				state = URI_AFTER_BACKSLASH;
			else
				state = URI_ERROR;
			break;
		case URI_AFTER_BACKSLASH:
			if (c == '%')
				state = URI_PERCENTAGE;
			else if (c == '.')
				state = URI_PATH_EXPANDER;
			else if (isValidPathIdentifier(c))
				state = URI_PATH_SEGMENT;
			else
				state = URI_ERROR;
			break;
		case URI_PATH_SEGMENT:
			if (c == '%')
				state = URI_PERCENTAGE;
			else if (c == '/')
				state = URI_AFTER_BACKSLASH;
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
				expandPath = true;
				state = URI_AFTER_BACKSLASH;
			}
			else if (pos + 1 < uriLen && c == '.' && req.uri[pos + 1] == '/')
			{
				expandPath = true;
				pos++;
				state = URI_AFTER_BACKSLASH;
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

	/* expand Path if neccessary */
	if (state == URI_PATH_SEGMENT)
		return SUCCESS;
	if (state == URI_EQUAL || state == URI_VALUE)
	{
		req.queryString = req.uri.substr(queryPos + 1);
		/* extract queries */
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

/* to be corrected */
static int parseHttpHeaderLine(HttpRequest &req)
{
	size_t p;
	while (!req.buffer.empty() && req.state != COMPLETE) {
		p = req.buffer.find("\r\n");
		if (p == std::string::npos)
			break;

		std::string line = req.buffer.substr(0, p);
		req.buffer.erase(0, p + 2);  // Remove line + CRLF

		if (line.empty()) {
			req.state = req.headers.count("Content-Length") ? BODY : COMPLETE;
			break;
		}

		size_t splitPos = line.find(":");
		if (splitPos != std::string::npos) {
			std::string key = line.substr(0, splitPos);
			std::string value = line.substr(splitPos + 2);
			req.headers[key] = value;
		}
	}
	return 0;
}

/* to be corrected */
static int parseHttpBody(HttpRequest &req)
{
	req.body.append(req.buffer);
	req.buffer.clear();

	if (req.headers.count("Content-Length"))
	{
		char *endptr = NULL;
		unsigned long contentLength = strtoul(req.headers["Content-Length"].c_str(), &endptr, 10);

		// Check if conversion was successful
		if (*endptr != '\0') {
			setRequestError(req, HTTP_BAD_REQUEST);
			return FAILURE;
		}

		if (req.body.size() >= contentLength)
		{
			req.state = COMPLETE;
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

    if (req.state == BODY)
        parseHttpBody(req);
}



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