#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include "httpbase.h"

#include <string>
#include <map>
#include <iostream>

class HttpResponse: public HttpBase {

public:
    HttpResponse(): 
        version(HttpVersion::v11), 
        status_code(HttpStatusCode::unknown),
        headers(),
        body() {}

    HttpResponse(int status, const std::string& body): 
        version(HttpVersion::v11),
        status_code(HttpStatusCode::unknown),
        headers(),
        body() {}
    
    inline void set_status(int code) {
        status_code = static_cast<HttpStatusCode>(code); 
    }

    inline std::string get_version() const {
        return version_to_str_map[version];
    }

    inline void append_to_body(const std::string& content) {
        body.append(content);
    }

    inline void append_to_body(const char* begin, const char* end) {
        body.append(begin, end);
    }
    
    inline void set_body(const std::string& _body) {
        body.assign(_body); // equal to operator=(const std::string&);
    }

    inline const std::string& get_body() const {
        return body;
    }

    inline void add_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    // return response
    const std::string& to_string_without_body();
    const std::string& to_string();

    void print(std::ostream& os);

private:
    HttpVersion version;
    HttpStatusCode status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif