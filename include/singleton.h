#ifndef __SINGLETON_H__
#define __SINGLETON_H__

// 梅耶单例模板类

#include <memory>

template<typename T>
class Singleton final {
public:

// 据说，返回指针存在被意外 delete 的风险，于是返回其引用。
    static T& GetInstance() {
        static T ins;
        return ins;
    }

private:
    Singleton() = default;
};

template<typename T>
class SingletonPtr {
public:

    using ptr = std::shared_ptr<T>;

    static ptr& GetInstance() {
        static ptr ins = std::make_shared<T>();
        return ins;
    }

private:
    SingletonPtr() = default;
};


#endif