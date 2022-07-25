#include "http.h"
// #include "noncopyable.h"
#include "exception.h"
#include "strtime.h"
#include "logger.h"
#include "helper.h"

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>

// 使用者唯一可以调用的方法
void Http::handle() {
    accept_request();
    handle_request();
    send_response();
}

void Http::accept_request() {
    char buf[2048];
    int n;

    // 在 connfd 连接符读数据
    n = Helper::readline(connfd, buf, sizeof(buf));
    // printf("message received.\n");
    INFO("message received: " + std::string(buf));

    char* beg = buf;
    char* end = beg + n;
    char* cur = beg;

    // http 请求最开始字段直到第一个空格应当是 method
    while (cur < end && !isspace(*cur)) cur ++;
    http_request.set_method(beg, cur);

    while (cur < end && isspace(*cur)) cur ++;

    // http 下一字段： path，path 后面可能是 ？ 做 query 分隔符
    beg = cur;
    while (cur < end && !isspace(*cur) && *cur != '?') cur ++;
    http_request.set_path(beg, cur);

    // 出现 ? 说明后面跟的是 query 内容
    if (*cur == '?') {
        beg = ++cur;
        while (cur < end && !isspace(*cur)) cur ++;
        http_request.set_query(beg, cur);
    }

    while (cur < end && isspace(*cur)) cur ++;

    // 下一字段： version
    beg = cur;
    while (cur < end && !isspace(*cur)) cur ++;
    http_request.set_version(beg, cur);

    while (cur < end && isspace(*cur)) cur ++;

    // 一行结束，下一系列字段： header: {key: value}, '\n' 换行，单独一行'\r\n' 结束
    while (true) {
        n = Helper::readline(connfd, buf, sizeof(buf));
        beg = buf;
        cur = beg;
        end = buf + n;

        while (cur < end && (*cur) != '\n') cur ++;
        if (*beg != '\r' && *(beg+1) != '\n')
            http_request.add_header(beg, cur);
        else break;

        // 此处的过滤空格实际没用，因为过滤完马上重新赋值
        while (cur < end && isspace(*cur)) cur ++;
    }

    // header 结束，后面是 body
    std::string content_length = http_request.get_header("Content-Length");
    if (content_length != "" ) {
        unsigned long length = std::stoul(content_length);
        while (length > 0) {
            n = ::read(connfd, buf, sizeof(buf));
            http_request.append_to_body(buf, buf+n);
            length -= n;
        }
    }

    request_flag = true;
    std::ofstream out_file;
    // out_file.open("/root/liux_httpd_server/log/tmplog.log", 
    //             std::ios::out | std::ios::trunc)
    http_request.print(std::cout);

    // out_file.close();
}


void Http::handle_request() {
    assert(request_flag);

    bool success;
    // 获取当前绝对路径
    std::string abs_path = absolute_path();

    if (!supported()) {
        // 501 不支持
        unimplemented();
        return ;
    }

    if (path_not_found(abs_path) && 
        http_request.get_method() != HttpMethod::put) {
        not_found();
        return;
    }

    // 请求的路径是否超出了系统根路径
    if (!Helper::urlUnderRootDir(http_request.get_path())) {
        not_found();
        return ;
    }

    if (is_directory(abs_path)) {
        if(!serve_index()) 
            index();
        return;
    }

    switch (http_request.get_method()) {
    // GET
    case HttpMethod::get:
        // if (!http_request.get_query().empty())
        //     success = execute_cgi(abs_path);
        // else 
        success = serve_file(abs_path);
        break;
    
    // HEAD
    case HttpMethod::head:
        // if (!http_request.get_query().empty())
        //     success = execute_cgi(abs_path);
        // else 
        success = serve_file(abs_path);
        break;
    
    // PUT
    case HttpMethod::put:
        success = serve_file(abs_path);
        break;
    
    // POST
    case HttpMethod::post:
        // success = execute_cgi(abs_path);
        // post 尚未实现，先给出一个接口，直接返回 true
        success = post_file(abs_path);
        break;
    
    // unknown;
    case HttpMethod::unknown:
        success = false;
        break;
    }

    // 500 服务器错误
    if (!success) 
        internal_server_error();
}

void Http::send_response() {
    std::string buf;
    StrTime now;
    // Weekday, day month year hour:min:second area
    http_response.add_header("Date", now.to_string("%a, %d %b %Y %H:%M:%S %Z"));
    const string& body = http_response.get_body();

    if (!body.empty()) 
        http_response.add_header("Content-Length", 
        std::to_string(http_response.get_body().size()));
    
    if (http_request.get_method() == HttpMethod::head)
        buf = http_response.to_string_without_body();
    else buf = http_response.to_string();

    write(connfd, buf.data(), buf.size());
}

const std::string& Http::absolute_path() {
    return HttpBase::HTTP_ROOT_DIR + http_request.get_path();
}

bool Http::is_directory(const std::string& path) {
    assert(request_flag);

    struct stat st;
    stat(path.data(), &st);
    return S_ISDIR(st.st_mode);
}

bool Http::path_not_found(const std::string& path) {
    assert(request_flag);

    struct stat st;
    return stat(path.data(), &st) == -1;
}

bool Http::supported() {
    assert(request_flag);
    return http_request.get_method() != HttpMethod::unknown
        && http_request.get_version() != HttpVersion::unknown;
}

// 返回目录下的 index.html 文件  
bool Http::serve_index() {
    const std::string& abs_path = absolute_path();
    std::string file;

    file = abs_path.back() == '/' ? abs_path + "index.html" :
                                    abs_path + "/index.html";
    if (!path_not_found(file)) {
        serve_file(file);
        return true;
    }
    return false;
}

// 返回静态文件
bool Http::serve_file(const std::string& path) {
    try {
        char buf[2048];
        std::ifstream f(path);
        // exception.h
        if (!f) throw ServeFileException(__FILE__, __LINE__, 
                "serve file open error: " + http_request.get_path());
        
        std::string suffix;

        int i = path.size() - 1;
        while (path[i] != '.' && path[i] != '/') i--;
        
        std::string content_type;
        if (path[i] == '/') content_type = "text/plain";
        else {  // path[i] == '.') 
            suffix = path.substr(i);
            auto it = HttpResponse::content_type_map.find(suffix);
            content_type = it == HttpResponse::content_type_map.end() 
                         ? "text/plain" : it -> second;
        }

        http_response.set_status_code(200);
        http_response.add_header("Content-type", content_type);

        while (1) { 
            f.read(buf, sizeof(buf));
            // gcount() 上一次读入的字符数
            if (f.gcount() == 0) break;
            http_response.append_to_body(buf, buf+f.gcount());
        }

        TRACE("suffix: " + suffix);
        response_flag = true;
        return true;
    } catch (ServeFileException e) {
        Logger::instance(LERROR) -> logging(e.filename, e.line_num, e.error_info);
        return false;
    }
}

bool Http::put_file(const std::string& path) {
    try {
        std::ofstream f;
        f.open(path);
        if (!f) throw PutFileException(__FILE__, __LINE__, 
                "put file open error: " + http_request.get_path());
        f << http_request.get_body();

        std::string url("http://");
        url += http_request.get_header("Host");
        url += http_request.get_path();
        http_response.set_body(url);

        return true;
    } catch (PutFileException e) {
        Logger::instance(LERROR) -> logging(e.filename, e.line_num, e.error_info);
        return false;
    }
}

// 先预留一个接口
bool Http::post_file(const std::string& path) {
    return true;
}

// 400 bad request 
void Http::bad_request()
{
    string body("");
     
    body += "<HTML><HEAD><TITLE>Bad Request</TITLE></HEAD>\r\n";
    body += "<BODY><P>400 Bad Request.</P>\r\n";
    body += "</BODY></HTML>\r\n";
    
    http_response.set_status_code(400);
    http_response.add_header("Content-Type", "text/html");
    http_response.set_body(body);
    
    response_flag = true;
}


// 404 Not found
void Http::not_found()
{
    string body("");
     
    body += "<HTML><HEAD><TITLE>Not Found</TITLE></HEAD>\r\n";
    body += "<BODY><P>404 NOT FOUND.</P>\r\n";
    body += "</BODY></HTML>\r\n";
    
    http_response.set_status_code(404);
    http_response.add_header("Content-Type", "text/html");
    http_response.set_body(body);
    
    response_flag = true;
    ERROR("not found: " + http_request.get_path());
}

// 500 HTTP-Internal Server Error
void Http::internal_server_error()
{
    string body("");
     
    body += "<HTML><HEAD><TITLE>HTTP-Internal Server Error</TITLE></HEAD>\r\n";
    body += "<BODY><P>500 HTTP-Internal Server Error.</P>\r\n";
    body += "</BODY></HTML>\r\n";
    
    http_response.set_status_code(500);
    http_response.add_header("Content-Type", "text/html");
    http_response.set_body(body);
    
    response_flag = true;
}

// 501 Not implemented
void Http::unimplemented()
{
    string body("");
     
    body += "<HTML><HEAD><TITLE>Method Not Implemented</TITLE></HEAD>\r\n";
    body += "<BODY><P>HTTP request method not supported.</P>\r\n";
    body += "</BODY></HTML>\r\n";
    
    http_response.set_status_code(501);
    http_response.add_header("Content-Type", "text/html");
    http_response.set_body(body);
    
    response_flag = true;
}

// index.html
void Http::index()
{
    string body("");
     
    body += "<HTML><HEAD><TITLE>Index</TITLE></HEAD>\r\n";
    body += "<BODY><P>Hello World.</P>\r\n";
    body += "</BODY></HTML>\r\n";
    
    http_response.set_status_code(200);
    http_response.add_header("Content-Type", "text/html");
    http_response.set_body(body);
    
    response_flag = true;
}
