#ifndef __HTTP_H__
#define __HTTP_H__

#include "httprequest.h"
#include "httpresponse.h"

class Http: public HttpBase {
public:
    Http(int _connfd): 
        connfd(_connfd), 
        request_flag(false),
        response_flag(false),
        request(),
        response() {}
    
    ~Http() {}

    // 处理事务
    void handle();

    // 从 socket 获取 http 请求并存入 http_request 变量
    void accept_request();

    // 处理请求  
    void handle_request();

    // 发送响应
    void send_response();

    // 资源的绝对路径
    const std::string& absolute_path();

    // 判断请求的是否是目录文件，必须在 path_not_found 之后调用
    bool is_directory(const string& path);

    // 判断请求的文件是否存在， 必须在 accept_request 之后调用
    bool path_not_found(const string& path);

    // 判断协议或函数是否被支持， 必须在 accept_request 之后调用
    bool supported();

    // 返回 index.html ，成功返回 true, 失败返回 false
    bool serve_index();

    // 返回静态文件，成功返回 true, 失败返回 false
    bool serve_index(); 

    // put 方法上传文件，成功返回 true, 失败返回 false
    bool put_file(const string& path)

    // 404
    void not_found();

    // 500
    void internal_serevr_error();

    // 501
    void uniplemented();

    // index
    void index()

private:
    int connfd;
    bool request_flag;
    bool response_flag;
    HttpRequest http_request;
    HttpResponse http_response;
    
};

#endif // __HTTP_H__