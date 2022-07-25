#ifndef __BLOCKINGQUEUE_H__
#define __BLOCKINGQUEUE_H__

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

// 一个线程安全的队列，每次操作前先加上独占锁

using std::queue;
using std::mutex;
using std::condition_variable;

template <typename T>
class BlockingQueue
{
public:
    using ptr = std::shared_ptr<BlockingQueue>;

    BlockingQueue() = default;
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    //
    // 队列尾部插入元素
    //
    void push_back(const T& v)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.push(v);
        _cond.notify_one(); // 唤醒等待队列中的第一个线程
    }
    
    //
    // 返回队列首部
    //
    const T& front()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_queue.empty())
            _cond.wait(lock);
        return _queue.front();
    }

    //
    // 队列首部删除元素
    //
    void pop_front()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_queue.empty())
            _cond.wait(lock);
        _queue.pop();
    }

    //
    // 队列元素个数
    //
    int size()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.size();
    }

    void clear() {
        std::unique_lock<std::mutex> lock(_mutex);
        queue<T> tmp;
        swap(tmp, _queue);
    }

private:
    queue<T> _queue;
    mutex _mutex;
    condition_variable _cond;
};

#endif