#ifndef __CHANNEL_H__
#define __CHANNEL_H__

// Channel 类负责将文件描述符 fd 和回调函数 callback 绑定在一起
// Channel 类负责将事件分发给注册在该文件描述符上的 callback 回调函数
// Channel 并不拥有一个文件描述符，不会再析构的时候关闭这个fd

#include <functional>
#include <memory>

class Channel {
public:
    using ptr = std::shared_ptr<Channel>;
    using EventCallback = std::function<void()>;
    
    Channel(int _fd): 
        fd(_fd), 
        enable_reading(false),
        enable_writing(false) {}

    // 处理事件
    void handle_event(){
        if (enable_reading && read_callback)  read_callback();
        if (enable_writing && write_callback) write_callback();
    }              

    inline int get_fd() const {return fd;} // 返回文件描述符
    inline bool is_enable_reading() const {return enable_reading;}
    inline bool is_enable_writing() const {return enable_writing;}

    inline void set_enable_reading() {enable_reading = true;}
    inline void set_enable_writing() {enable_writing = true;}

    inline void set_read_callback(const EventCallback& cb) {
        read_callback = cb;
    }
    inline void set_write_callback(const EventCallback& cb) {
        write_callback = cb;
    }
    inline void set_error_callback(const EventCallback& cb) {
        error_callback = cb;
    }

private:
    int fd;
    bool enable_reading;
    bool enable_writing;

    EventCallback read_callback;
    EventCallback write_callback;
    EventCallback error_callback;
};

#endif