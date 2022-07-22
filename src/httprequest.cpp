#include "httprequest.h"
#include <algorithm>

void HttpRequest::set_method(const char* start, const char* end) {
    std::string m(start, end);
    auto it = str_to_method_map.find(m);
    method = it == str_to_method_map.end() ?
             HttpMethod::unknown : it -> second;
}

void HttpRequest::set_version(const char* start, const char* end) {
    std::string v(start, end);
    auto it = str_to_version_map.find(v);
    version = it == str_to_version_map.end() ? 
              HttpVersion::unknown : it -> second;
}

void HttpRequest::set_path(const char* start, const char* end) {
    path.assign(start, end);
}

void HttpRequest::set_query(const char* start, const char* end) {
    query.assign(start, end);
}

void HttpRequest::add_header(const char* start, const char* end) {
    const char* p = std::find(start, end, ':');
    std::string key(start, p);
    
    p ++;
    while (p < end && isspace(*p)) p ++;
    const char* e = end;
    while (e > p && isspace(*e)) e--;
    std::string value(p, e+1);

    headers[key] = value;
}

std::string HttpRequest::get_header(const std::string& key) {
    std::string result;
    auto it = headers.find(key);
    if (it != headers.end())
        result = it -> second;
    return result;
} 

void HttpRequest::print(std::ostream& os) {
    os << ">" << method_to_str_map[mehtod];
    os << " " << path;
    if (query.size()) os << "?" query;
    os << " ";

    os << get_str_version() << "\r\n";

    for (const auto& header: headers) {
        os << ">";
        os << header.first << ": " <<header.second << "\r\n";
    }

    os << "\r\n";
    os << ">" << body << std::endl;
}