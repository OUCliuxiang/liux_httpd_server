#ifndef __TCP_H__
#define __TCP_H__

#include "eventloop.h"
#include "channel.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <functional>

#include <mutex>
#include <vector>
#include <list>
#include <map>

template<typename Protocol, typename IOMulti> // 目前只实现了 Http 协议
class TcpServer {
public:
    using callback = std::function<void()>;

    TcpServer(uint32_t _port, int _threads): 
        port(_port),
        threads(_threads),
        sockfd(setup(port)) {}
    
    ~TcpServer() {
        for (auto p: eventloops) delete p;
        close(sockfd);
    }

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    // 运行 tcp 
    void run();


private:
    // 初始化。创建 socket, 将 socket 绑定 ip:port，并监听
    // 返回监听文件描述符
    int setup(int _port);
    // 处理请求返返回响应
    void handle(int connfd);
    // 线程池
    void make_threads();
    // 封装 accept，返回连接 connfd 文件描述符
    int accept(int fd, struct sockaddr* addr, socklen_t* len);
    // 监听 channel 
    void listen_channel(int listenfd, EventLoop* eventloop);
    // 事件循环
    void loop(EventLoop* eventloop);
    // 删除超时连接
    void clean_channel(EventLoop* eventloop);


private:
    uint32_t port;
    int threads;
    int sockfd;
    std::mutex mutex;
    std::vector<EventLoop*> eventloops;
    std::map<EventLoop*, std::list<Channel*>> closing_list;
};


#endif // __TCP_H__