#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__

// IO 多路复用

#include <vector>
#include <map>

#include "channel.h"

// 获取 IO 事件
class EventLoop {
public:
    using ChannelList = std::vector<Channel*>;

    explicit EventLoop(int listenfd): socketfd(listenfd) {}
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void add_channel(int fd, Channel* channel); // 添加 channel 对象
    void del_channel(int fd);                   // 删除 channel 对象

    // 将获取的事件放入 channel_list 中以参数形式返回
    virtual void loop(ChannelList& channel_list) = 0;
    virtual ~EventLoop() = 0;


protected:
    int socketfd;
    ChannelList* active_channel_list;
    std::map<int, Channel*> channel_map;       // fd --> channel 的映射
};


// select IO 多路复用
class Select: public EventLoop {
public:
    virtual void loop(ChannelList& channel_list) override;
};


// poll IO 多路复用
class Poll: public EventLoop {
public:
    virtual void loop(ChannelList& channel_list) override;
};


// epoll IO 多路复用    
class Epoll: public EventLoop {
public:
    virtual void loop(ChannelList& channel_list) override;
};

#endif