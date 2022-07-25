#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

// IO 多路复用

#include <vector>
#include <map>
#include "channel.h"
#include <memory>

// 获取 IO 事件
class EventLoop {
public:
    using ptr = std::shared_ptr<EventLoop>;
    using ChannelList = std::vector<Channel::ptr>;

    EventLoop(int listenfd): sockfd(listenfd) {}
    EventLoop() {}
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    inline void set_sockfd(int listenfd) {
        sockfd = listenfd;
    }

    void add_channel(Channel::ptr channel); // 添加 channel 对象
    void del_channel(Channel::ptr channel); // 删除 channel 对象

    // 将获取的事件放入 channel_list 中以参数形式返回
    virtual void loop() = 0;
    // 析构不要纯虚，除非必须且派生类实际给出了其具体定义。
    // 这里如果不实现只定义，编译将不通过，报错 undefined reference
    virtual ~EventLoop() {}


protected:
    int sockfd;
    ChannelList channel_list;
    std::map<int, Channel::ptr> channel_map;       // fd --> channel 的映射
};


// select IO 多路复用
class Select: public EventLoop {
public:
    using ptr = std::shared_ptr<Select>;
    Select() {};
    virtual void loop() override;
    // ~Select() {} // 基类声明为虚函数，这里一定要实现
};


// poll IO 多路复用
class Poll: public EventLoop {
public:
    using ptr = std::shared_ptr<Poll>;
    Poll() {}
    virtual void loop() override;
    // ~Poll() {} // 基类声明为虚函数，这里一定要实现
};


// epoll IO 多路复用    
class Epoll: public EventLoop {
public:
    using ptr = std::shared_ptr<Epoll>;
    Epoll() {}
    virtual void loop() override;
    // ~Epoll() {} // 基类声明为虚函数，这里一定要实现
};

#endif