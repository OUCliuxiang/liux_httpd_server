#ifndef __TCP_H__
#define __TCP_H__

#include "eventloop.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <functional>
#include <unistd.h>

#include <mutex>
#include <vector>
#include <list>
#include <map>
#include <memory>

// !!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__
// _!_!_!_!_!_!_!_!_!_!_ 模板类的定义和实现不要分开写，我操 !_!_!_!_!_!_!_!_!_!_!_!_!
// !!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__!!__
// 这是由于 template 指定的模板类型在定义阶段，编译器不为其分配空间。
// 上面是 C++ 编程思想的解释，不理解，先记住不能分开写。
// 但我想隐藏代码，不写模板类了

// template<typename Protocol, typename Multi = Epoll> // 目前只实现了基本可用的 Http 协议
class TcpServer {
public:
    using ptr = std::shared_ptr<TcpServer>;
    using callback = std::function<void()>;

    TcpServer(uint32_t _port, int _threads, int _iotype = 2): 
        port(_port),
        threads(_threads),
        iotype(_iotype),
        sockfd(setup(_port)) {}
    
    ~TcpServer() {
        // 用智能指针实现，这个不需要了
        // for (auto p: eventloops) delete p;
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
    void listen_channel(int listenfd, EventLoop::ptr eventloop);
    // 事件循环
    void loop(EventLoop::ptr eventloop);
    // 删除超时连接
    void clean_channel(EventLoop::ptr eventloop);


private:
    uint32_t port;
    int threads;
    int iotype;

    int sockfd;

    std::mutex mutex;
    std::vector<EventLoop::ptr> eventloops;
    std::map<EventLoop::ptr, std::list<Channel::ptr>> closing_list;
};


#endif // __TCP_H__