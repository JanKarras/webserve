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
