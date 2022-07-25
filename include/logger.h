#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "blockingqueue.h"
#include "helper.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#define LEVEL_SIZE  5

#define LTRACE   0
#define LDEBUG   1
#define LINFO    2
#define LWARN    3
#define LERROR   4


#define TRACE(content)  Logger::instance(LTRACE) -> logging(__FILE__, __LINE__, content)
#define DEBUG(content)  Logger::instance(LDEBUG) -> logging(__FILE__, __LINE__, content)
#define INFO(content)   Logger::instance(LINFO) -> logging(__FILE__, __LINE__, content)
#define WARN(content)   Logger::instance(LWARN) -> logging(__FILE__, __LINE__, content)
#define ERROR(content)  Logger::instance(LERROR) -> logging(__FILE__, __LINE__, content)

//
// 日志类
//
class Logger
{
public:
    using ptr = std::shared_ptr<Logger>;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    // 每个级别一个实例。
    static ptr instance(int level);
    
    // 回收内存
    // 使用智能指针，这个不需要了
    // static void destroy();
    
    // 执行实际的写日志操作,在独立的线程中调用
    static void thread_func();

    // 添加至待记录日志队列
    static void append(const string& log);

    // 返回日志级别的字符串
    const std::string& strLevel();

    // 检测该日志是否高于当前该记录的日志级别
    // 并做日志记录
    void logging(const string& file, int line, const string& content);

private:
    
    Logger(int level = 1): _level(level) {
        std::cout << "level: " << _level << std::endl;
    }

private:

    int _level;                     // 要输出的日志级别

    static char seperator;              // 日志分隔符
    static int _curruct_level;          // 当前日志级别，用作过滤阈值
    static std::ostream& _output;       // 日志输出流
    // static std::ofstream& _output_file; // 日志输出文件
    
    // 级别到字符串的映射表
    static std::vector<std::string> _strLevelMap;   
    // 实例数组，每个级别一个实例
    static std::vector<ptr> _instances;
    // 阻塞队列
    static BlockingQueue<std::string> _queue;            
    
};
#endif