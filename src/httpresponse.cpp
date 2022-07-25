#include "httpresponse.h"
#include <algorithm>

const std::string& HttpResponse::to_string_without_body() {
    std::string response;
    response += version_to_str_map[version];
    response += " ";
    response += status_to_str_map[status_code];
    response += "\r\n";

    for (const auto& it: headers) {
        response += it.first;
        response += ": ";
        response += it.second;
        response += "\r\n";
    }
    response += "\r\n";
    return response;
}

const std::string& HttpResponse::to_string() {
    std::string response = to_string_without_body();
    response += body;
    return response;
}

void HttpResponse::print(std::ostream& os) {
    std::string response = to_string_without_body();
    auto begin = response.begin();
    auto current = begin;
    auto end = response.end();

    for (; current != end; ++current) {
        if (*current == '\r' && *(current+1) == '\n') {
            os << "<";
            os << std::string(begin, current+2);
            if (begin == current) break;
            else begin = current + 2;
        }
    }

    os << std::endl;
}