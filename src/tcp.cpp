#include "tcp.h"
#include "logger.h"
#include "helper.h"
#include "http.h"
#include <stdio.h>

#include <thread>
#include <string.h>
#include <unistd.h> // close()
#include <stdlib.h> // pause()

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
    
    // 创建一个 TCP (SOCK_STREAM) 连接
    int _sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_sockfd < 0) {
        ERROR("socket create failed");
        printf("socket create failed\n");
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
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        ERROR("set socket opt failed: SO_REUSEADDR");
        printf("set socket opt failed: SO_REUSEADDR\n");
        exit(1);
    }

    // bind & listen，绑定 sockfd 并监听
    struct sockaddr_in seraddr;
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(_port);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(_sockfd, (struct sockaddr*)(&seraddr), sizeof(seraddr)) < 0) {
        ERROR("bind failed");
        printf("bind failed: \n");
        exit(1);
    }

    // 第二个参数是 accept 之前的队列，当超过这个数量的队列没有被 accept
    // 继续到来的连接将会被拒绝连接。
    if (listen(_sockfd, 5) < 0) {
        ERROR("listen failed");
        printf("listen failed\n");
        exit(1);
    }

    return _sockfd;
}


void TcpServer::handle(int connfd) {
    INFO("new request: " + std::to_string(connfd));
    Http::ptr http(new Http(connfd)); // 目前只实现了 http 协议    
    http -> handle();
}

// 线程池，将每一个 Loop 作为单独一个线程剥离出去，抢占式调度
void TcpServer::make_threads() {
    // mem_fn 成员函数指针包装器
    auto fcn = std::mem_fn(&TcpServer::loop);
    for (int i = 0; i < threads; i++) {
        // EventLoop::ptr eventloop(nullptr);
        if (iotype == 0) {
            Select::ptr eventloop(new Select());
            eventloop -> set_sockfd(sockfd);
            closing_list[eventloop] = std::list<Channel::ptr>();

            listen_channel(sockfd, eventloop);
            eventloops.push_back(eventloop);

            std::thread loop_thread(fcn, this, eventloop);
            loop_thread.detach();
        }

        else if (iotype == 1) {
            Poll::ptr eventloop(new Poll());
            eventloop -> set_sockfd(sockfd);
            closing_list[eventloop] = std::list<Channel::ptr>();

            listen_channel(sockfd, eventloop);
            eventloops.push_back(eventloop);

            std::thread loop_thread(fcn, this, eventloop);
            loop_thread.detach();
        }
        
        else if (iotype == 2) {
            Epoll::ptr eventloop(new Epoll());
            eventloop -> set_sockfd(sockfd);
            closing_list[eventloop] = std::list<Channel::ptr>();

            listen_channel(sockfd, eventloop);
            eventloops.push_back(eventloop);

            std::thread loop_thread(fcn, this, eventloop);
            loop_thread.detach();
        }

        else {
            ERROR("value iotype must be 0, 1, or 2, now is " + std::to_string(iotype));
            exit(1);
        }
        
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
void TcpServer::listen_channel(int listenfd, EventLoop::ptr eventloop) {
    Channel::ptr channel(new Channel(listenfd));
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
              client_addr + ":" + std::to_string(cliaddr.sin_port));
        
        // 10 秒计时器
        int timerfd = Helper::timerfd_create(10);

        Channel::ptr connect_channel(new Channel(connfd));
        connect_channel -> set_enable_reading();
        // 同样，lambda 函数形式添加回调事件
        connect_channel -> set_read_callback([=](){
            handle(connfd);
        });
        eventloop -> add_channel(connect_channel);

        Channel::ptr timer_channel(new Channel(timerfd));
        timer_channel -> set_enable_reading();
        timer_channel -> set_read_callback([=](){
            closing_list[eventloop].push_back(connect_channel);
            closing_list[eventloop].push_back(timer_channel);
            INFO("connection close: " + std::to_string(connfd));
        });
        eventloop -> add_channel(timer_channel);
    });
    eventloop -> add_channel(channel);
}

void TcpServer::loop(EventLoop::ptr eventloop) {
    while (true) {
        eventloop -> loop();
        clean_channel(eventloop);
    }
} 


void TcpServer::clean_channel(EventLoop::ptr eventloop) {
    for (auto it = closing_list[eventloop].begin(); 
    it != closing_list[eventloop].end(); ) {
        Channel::ptr channel = (*it);
        it = closing_list[eventloop].erase(it);
        eventloop -> del_channel(channel);
        close(channel -> get_fd());
        // delete channel;
    }
}