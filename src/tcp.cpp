#include "tcp.h"
#include "logger.h"
#include "helper.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void TcpServer::run() {
    // 日志单独一个线程
    std::thread log_thread(Logger::thread_func);
    // 日志子线程剥离出主线程而独自运行
    log_thread.detach();
    // 创建线程池，开始运行
    make_threads();
    while(1) pause();
}

int TcpServer::setup(int _port) {
    
    // 创建一个 TCP 连接
    int _sockfd = socket(AF_INET, SOKC_STREAM, 0);
    if (_sockfd < 0) {
        ERROR("socket create failed");
        exit(1);
    }

    /** 
    setsockopt(                
        int socket,                 套接字描述符
        int level,                  级别，常设置为 SOL_SOCKET 套接字级别
        int option_name,            选项，可以用按位与操作符 | 进行设置多选项
        const void* option_value,   没有细究，通常给如一个 int 型值为 1 的变量地址
        size_t option_len)          没有细究，上一个选项的长度，则直接 sizeof(int)
    */ 
    int flag = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0) {
        ERROR("set socket opt failed: SO_REUSEADDR");
        exit(1);
    }

    // bind & listen，绑定 sockfd 并监听
    struct sockaddr_in seraddr;
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(_port);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*)(&seraddr), sizeof(seraddr)) < 0) {
        ERROR("bind failed");
        exit(1);
    }

    // 第二个参数是 accept 之前的队列，当超过这个数量的队列没有被 accept
    // 继续到来的连接将会被拒绝连接。
    if (listen(sockfd, 5) < 0) {
        ERROR("listen failed");
        exit(1);
    }

    return _sockfd;
}

void TcpServer::handle(int connfd) {
    INFO("new request: " + std::to_string(connfd));
    Protocol protocol(connfd); // 目前只实现了 http 协议    
    protocal.handle();
}

void TcpServer::make_threads() {
    // mem_fn 成员函数指针包装器
    auto fcn = std::mem_fn(&TcpServer<Protocol, Multi>::loop);
    for (int i = 0; i < threads; i++) {
        EventLoop* eventloop = new Multi(sockfd);
        closing_list[eventloop] = std::list<Channel*>();

        listen_channel(sockfd, eventloop);
        eventloops.push_back(eventloop);

        std::thread loop_thread(fcn, this, eventloop);
        loop_thread.detach();
    }
}


int TcpServer::accept(int fd, struct sockaddr* addr, socklen_t* len) {
    // 对互斥量加独占锁
    std::unique_lock<std::mutex> lock(mutex);
    int connfd = ::accept(fd, addr, len);
    if (connfd < 0) {
        ERROR("accept failed");
        exit(1);
    }
    return connfd;
}

// 监听 channel 
void TcpSerevr::listen_channel(int listenfd, EventLoop* eventloop) {
    Channel* channel = new Channel(listenfd);
    channel -> set_enable_reading();
    // 匿名函数方法添加读事件回调函数，按值捕获所有变量
    channel -> set_read_callback([=](){
        char client_addr[32];
        struct sockaddr_in cliaddr;
        socklen_t length;

        // accept
        int connfd = accept(channel -> get_fd(), (struct sockaddr*)&cliaddr, &length);
        inet_ntop(AF_INET, &cliaddr.sin_addr, client_addr, sizeof(client_addr));
        INFO("new connection: " + std::to_string(connfd) + " : " + 
              client_addr + ":" + to_string(cliaddr.sin_port));
        
        // 10 秒计时器
        int timerfd = Helper::timerfd_create(10);

        Channel* connect_channel = new Channel(connfd);
        connect_channel -> set_enable_reading();
        // 同样，lambda 函数形式添加回调事件
        connect_channel -> set_read_callback([=](){
            handle(connfd);
        });
        eventloop -> add_channel(connect_channel);

        Channel* timer_channel = new Channel(timerfd);
        timer_channel -> set_enable_reading();
        timer_channel -> set_read_callback(()[=]{
            closing_list[eventloop].push_back(connect_channel);
            closing_list[eventloop].push_back(timer_channel);
            INFO("connection close: " + std::to_string(connfd));
        });
        eventloop -> add_channel(timer_channel);
    });
    eventloop -> add_channel(channel);
}

void TcpServer::loop(EventLoop& eventloop) {
    while (true) {
        eventloop -> loop();
        clean_channel(eventloop);
    }
} 


void TcpServer::clean_channel(EventLoop* eventloop) {
    for (auto it: closing_list[eventloop].begin(); 
    it != closing_list[eventloop].end(); ) {
        Channel* channel = (*it);
        it = closing_list[eventloop].erase(it);
        eventloop -> del_channel(channel);
        close(channel -> get_fd());
        delete channel;
    }
}