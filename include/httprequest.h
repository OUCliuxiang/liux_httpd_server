#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include "HttpBase.h"
#include <string>
#include <map>
#include <iostream>

class HttpRequest: public HttpBase {

public:
    
    HttpRequest():
        method(HttpMethod::unknown),
        version(HttpVersion::unknown),
        path(),
        query(),
        headers(),
        body() {}

    HttpRequest(HttpMethod _method, HttpVersion _version):
        method(_method),
        version(_version),
        path(),
        query(),
        headers(),
        body() {}

    HttpRequest(HttpMethod _method, HttpVersion _version , 
                const std::string& _path, const std::stirng& _query):
        method(_method),
        version(_version),
        path(_path),
        query(_query),
        headers(),
        body() {}
    
    void set_method(const char* start, const char* end);
    inline HttpMethod get_method() {
        return method;
    }
    inline std::string get_str_method() {
        return method_to_str_map[method];
    }

    void set_version(const char* start, const char* end);
    inline HttpVersion get_version() {
        return version;
    }
    inline std::string get_str_version() {
        return version_to_str_map[version];
    }

    void set_path(const char* start, const char* end);
    inline std::string get_path() {
        return path;
    }

    void set_query(const char* start, const char* end);
    inline std::string get_query() {
        return query;
    }

    void add_header(const char* start, const char* end);
    std::string get_header(const std::string& key);

    inline void append_to_body(const std::string& content) {
        body.append(content);
    }
    inline void append_to_body(const char* strat, const char* end) {
        body.append(start, end);
    }

    inline const std::string& get_body() {
        return body;
    }
    
    void print(std::ostream& os);


private:
    HttpMethod method;
    HttpVersion version;
    std::string path;
    std::string query;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif // __HTTPREQUEST_H__