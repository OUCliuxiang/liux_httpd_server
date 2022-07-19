#include "eventloop.h"
#include "logger.h"

#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>

#include <assert.h>


void EventLoop::add_channel(Channel* channel) {
    channel_map[channel -> fd] = channel;
    TRACE("chanel add to EventLoop: " + std::to_string(channel -> fd));
}

void EventLoop::del_channel(Channel* channel) {
    auto it = channel_map.find(channel -> fd);
    if (it != channel_map.end()) {
        channel_map.erase(it);
        TRACE("channel remove from EventLoop: " + std::to_string(channel -> fd));
    } else {
        WARN(channel -> fd + " does not exist in EventLoop.")
    }
}


void Select::loop(ChannelList& channel_list) {
    fd_set rset; // 记录读事件的 fd_set 数组
    
    // 构造 rset 数组
    FD_ZERO(&set);
    for (auto& x: channel_map) 
        if ((x.second) -> enable_reading())
            FD_SET(x.first, set);

    assert(channel_map.size());
    int maxfd = (-- channel_map.end()) -> first;

    // 调用全局函数
    // 其余三个 NULL 分别对应写事件、异常事件、超时时间。
    // 超时时间是一个 timeval 结构体，包含 tv_sec 秒数与 tv_usec 微秒数
    // 此处 null 代表一直阻塞。
    int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
    for (auto& x: channel_map) {
        if (FD_ISSET(x.first, &rset)) {
            channel_list.push_back(x.second);
            if (-- nready <= 0) break;
        }
    }
}

void Poll::loop(ChannelList& channel_list) {
    std::vector<struct pollfd> fd_array; // 记录读事件的 poll 结构体数组
    // 构造 fd_array 数组
    for (auto& x: channel_map) {
        if (x.second -> enable_reading()) {
            struct pollfd poll_fd;
            poll_fd.fd = x.first;
            poll_fd.events = POLLRDNORN;
            fd_array.push_back(poll_fd);
        }
    }

    // fd_array.data() 先从 vector 数组转为普通 pollfd 类型数组
    // static_cast<> 获取 pollfd 类型数组头指针
    // 最后一个 -1 代表超时时间为一直阻塞
    int nready = poll(static_cast<struct pollfd*>(fd_array.data()), fd_array.size(), -1);
    for (auto& x: fd_array) {
        // 位与运算判断是否返回了可读
        if (x.revents & POLLRDNORN) {
            channel_list.push_back(channel_map[x.fd]);
            if (--nready <= 0) break;
        }
    }

}

void Epoll::loop(ChannelList& channel_list) {
    int max_events = 1000;
    struct epoll_event ev, events(max_events);

    int epollfd = epoll_create(10);
    if (epollfd == -1) {
        ERROR("epoll_create failed");
        exit(1);
    } 

    for (auto& x: channel_map) {
        if (x.second -> enable_reading()) {
            ev.events = EPOLLIN;
            ev.data.fd = x.first;
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, x.first, &ev) == -1){
                ERROR("epoll_ctl (add) faied");
                exit(1);
            }
        }
    }

    int nready = epoll_wait(epollfd, events, max_events, -1);
    if (nready == -1) {
        ERROR("epoll_wait failed");
        exit(1);
    }
    
} 


