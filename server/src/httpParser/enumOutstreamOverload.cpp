#include "../../include/webserv.hpp"

std::ostream& operator<<(std::ostream& os, const HttpMethod& method) {
    switch (method) {
        case GET:
            os << "GET";
            break;
        case POST:
            os << "POST";
            break;
        case DELETE:
            os << "DELETE";
            break;
        case HEAD:
            os << "HEAD";
            break;
        case PUT:
            os << "PUT";
            break;
        case CONNECT:
            os << "CONNECT";
            break;
        case OPTIONS:
            os << "OPTIONS";
            break;
        case TRACE:
            os << "TRACE";
            break;
        case PATCH:
            os << "PATCH";
            break;
        case INVALID:
            os << "INVALID";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const RequestState& state) {
    switch (state) {
        case REQUEST_LINE:
            os << "REQUEST_LINE";
            break;
        case HEADERS:
            os << "HEADERS";
            break;
        case BODY:
            os << "BODY";
            break;
		case NO_BODY:
			os << "NO_BODY";
			break;
		case BODY_CHUNKED:
			os << "BODY_CHUNKED";
			break;
        case COMPLETE:
            os << "COMPLETE";
            break;
        case ERROR:
            os << "ERROR";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

// Overloading the << operator to print the enum as a string
std::ostream& operator<<(std::ostream& os, const RequestLineState& state) {
    switch (state) {
        case RL_START:
            os << "RL_START";
            break;
        case RL_METHOD:
            os << "RL_METHOD";
            break;
        case RL_URI:
            os << "RL_URI";
            break;
        case RL_VERSION:
            os << "RL_VERSION";
            break;
        case RL_DONE:
            os << "RL_DONE";
            break;
        case RL_ERROR:
            os << "RL_ERROR";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

// Overloading the << operator to print the enum as a string
std::ostream& operator<<(std::ostream& os, const BodyState& state) {
    switch (state) {
        case B_CHUNK_SIZE_START:
            os << "CHUNK_SIZE_START";
            break;
        case B_CHUNK_SIZE:
            os << "B_CHUNK_SIZE";
            break;
        case B_CHUNK_SIZE_CRLF:
            os << "B_CHUNK_SIZE_CRLF";
            break;
        case B_CHUNK_DATA_START:
            os << "B_CHUNK_DATA_START";
            break;
        case B_CHUNK_DATA:
            os << "B_CHUNK_DATA";
            break;
        case B_CHUNK_DATA_CRLF:
            os << "B_CHUNK_DATA_CRLF";
            break;
        case B_CHUNK_TRAILER_START:
            os << "B_CHUNK_TRAILER_START";
            break;
        case B_CHUNK_TRAILER:
            os << "B_CHUNK_TRAILER";
            break;
        case B_CHUNK_TRAILER_CRLF:
            os << "B_CHUNK_TRAILER_CRLF";
            break;
        case B_FINAL_CRLF:
            os << "B_FINAL_CRLF";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}