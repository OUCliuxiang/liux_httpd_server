#ifndef __HELPER_H__
#define __HELPER_H__

#include <string>

using std::string;

namespace Helper
{

    //
    // 读取一行数据
    //
    int readline(int fd, char *buf, int size);
    
    //
    // 当前日期字符串
    //
    string date();

    //
    // 当前时间字符串
    //
    string time();
    
    //
    // 返回url的相对深度
    // 根据状态机计算是否超出根目录
    //
    bool urlUnderRootDir(const string& url);
    
    //
    // 获取一个随时间变化的随机字符串
    // random-string = Hash(time-string ":" private-key)
    //
    string random_string(int second);

    // 计时器
    int timerfd_create(int second);
}

#endif