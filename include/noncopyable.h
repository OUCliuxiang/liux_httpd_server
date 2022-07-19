#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__

class Noncopyable {
public:
    Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
    Noncopyable(Noncopyable&&) = delete;
}

#endif