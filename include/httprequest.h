#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include "httpbase.h"
#include <string>
#include <map>
#include <iostream>

class HttpRequest: public HttpBase {

public:
    using ptr = std::shared_ptr<HttpRequest>;
    
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
                const std::string& _path, const std::string& _query):
        method(_method),
        version(_version),
        path(_path),
        query(_query),
        headers(),
        body() {}
    
    void set_method(const char* start, const char* end);
    inline HttpMethod get_method() const {
        return method;
    }
    inline const std::string& get_str_method() const {
        return method_to_str_map[method];
    }

    void set_version(const char* start, const char* end);
    inline HttpVersion get_version() const {
        return version;
    }
    inline const std::string& get_str_version() const {
        return version_to_str_map[version];
    }

    inline void set_path(const char* start, const char* end) {
        path.assign(start, end);
    }
    inline const std::string get_path() const {
        return path;
    }

    inline void set_query(const char* start, const char* end) {
        query.assign(start, end);
    }
    inline const std::string& get_query() const {
        return query;
    }

    void add_header(const char* start, const char* end);
    const std::string get_header(const std::string& key) const;

    inline void append_to_body(const std::string& content) {
        body.append(content);
    }
    inline void append_to_body(const char* start, const char* end) {
        body.append(start, end);
    }

    inline const std::string& get_body() const {
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