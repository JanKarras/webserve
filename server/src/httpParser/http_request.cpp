#include "../../include/webserv.hpp"

static void setRequestError(HttpRequest &req, int errType)
{
	req.buffer.clear();
	req.state = ERROR;
	req.exitStatus = errType;
}

static int parseQueryString(HttpRequest &req)
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
}

static int parseHttpRequestLine(HttpRequest &req)
{
	size_t p = req.pos;
	size_t uriLength;
	RequestLineState state = static_cast<RequestLineState>(req.parseState);
	// std::cout << "request section status: " << req.state << ",status: " << state << ", buffer: " << req.buffer << std::endl;
	size_t buffer_length = req.buffer.length();
	// std::cout << "Position: " << p << std::endl;
	// std::cout << "Buffer_length: " << buffer_length << std::endl;
	for (; p < buffer_length; p++)
	{
		// std::cout << "Status: " << state << std::endl;
		// std::cout << "Position: " << p << std::endl;
		u_char c = req.buffer[p];
		// std::cout << "Character: " << c << std::endl;

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

/* parse URI ... */


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
