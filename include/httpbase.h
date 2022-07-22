#ifndef __HTTPBASE_H__
#define __HTTPBASE_H__

#include <vector>
#include <map>
#include <string>

class HttpBase {
public:
    // http root dir
    static std::string HTTP_ROOT_DIR;
    // http method
    enum class HttpMethod {
        unknown, get, head, put, post
    };
    // http version 
    enum class HttpVersion {
        unknown, v09, v10, v11, v20
    };
    // http status code 
    enum class HttpStatusCode {
        unknown = 0,
        s200    = 200,
        s301    = 301,
        s400    = 400,
        s401    = 401,
        s404    = 404,
        s500    = 500,
        s501    = 501
    };

    static std::map<HttpMethod, std::string> method_to_str_map;
    static std::map<std::string, HttpMethod> str_to_method_map;

    static std::map<HttpVersion, std::string> version_to_str_map;
    static std::map<std::string, HttpVersion> str_to_version_map;

    static std::map<HttpStatusCode, std::string> status_to_str_map;

    static std::map<std::string, std::string> content_type_map;

    static std::vector<char> url_charset;

    // set void set_root_dir
    static set_void_dir(const std::string& dir) {
        HTTP_ROOT_DIR = dir;
    }

};

#endif // __HTTPBASE_H__