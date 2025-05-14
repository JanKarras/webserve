#include "../../include/webserv.hpp"

int parseCgiContent(HttpResponse &res) {
	std::cout << res.body << std::endl;
	std::string		key, value;
	size_t			pos = 0;
	uint8_t			ch;
	CgiParseState	state = CGI_START_LINE;

	for (; pos < res.body.size(); pos++)
	{
		ch = res.body[pos];
		switch(state)
		{
			case(CGI_START_LINE):
				key.erase();
				value.erase();
				if (ch == '\r')
					state = CGI_HEADERS_END;
				else if (isalnum(ch))
				{
					key.push_back(ch);
					state = CGI_KEY;
				}
				else
				{
					Logger::error("Malformed header from cgi script");
					return FAILURE;
				}
				break;
			case(CGI_KEY):
				if (ch == ':')
					state = CGI_BEFORE_VALUE;
				else if (!isalnum(ch) && ch != '-')
				{
					Logger::error("Malformed header key from cgi script");
					return FAILURE;
				}
				else
					key.push_back(ch);
				break;
			case(CGI_BEFORE_VALUE):
				if (ch == ' ')
					break;
				else if (ch == '\r')
					state = CGI_CRLN;
				else if (ch >= 32 && ch <= 126)
				{
					value.push_back(ch);
					state = CGI_VALUE;
				}
				else
				{
					Logger::error("Malformed header value from cgi script");
					return FAILURE;
				}
				break;
			case(CGI_VALUE):
				if (ch == '\r')
					state = CGI_CRLN;
				else if (ch >= 32 && ch <= 126)
				{
					value.push_back(ch);
					state = CGI_VALUE;
				}
				else
				{
					Logger::error("Malformed header value from cgi script");
					return FAILURE;
				}
				break;
			case(CGI_CRLN):
				if (ch == '\n')
				{
					res.headers[key] = value;
					state = CGI_START_LINE;
				}
				else
				{
					Logger::error("Malformed header from cgi script");
					return FAILURE;
				}
				break;
			case(CGI_HEADERS_END):
				if (ch == '\n')
				{
					res.body.erase(0, pos + 1);
					return SUCCESS;
				}
				else
				{
					Logger::error("Malformed header from cgi script");
					return FAILURE;
				}
				break;
		}
	}
	Logger::error("Unexpected EOF in CGI headers");
	return FAILURE;
}
