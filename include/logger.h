#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "blockingqueue.h"
#include "helper.h"
#include <iostream>
#include <string>
#include <vector>

#define LEVEL_SIZE  5

#define TRACE   0
#define DEBUG   1
#define INFO    2
#define WARN    3
#define ERROR   4


#define TRACE(content)  Logger::instance(TRACE) -> logging(__FILE__, __LINE__, content)
#define DEBUG(content)  Logger::instance(DEBUG) -> logging(__FILE__, __LINE__, content)
#define INFO(content)   Logger::instance(INFO) -> logging(__FILE__, __LINE__, content)
#define WARN(content)   Logger::instance(WARN) -> logging(__FILE__, __LINE__, content)
#define ERROR(content)  Logger::instance(ERROR) -> logging(__FILE__, __LINE__, content)

//
// 日志类
//
class Logger
{
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    // 每个级别一个实例。
    static Logger* instance(int level);
    
    // 回收内存
    static void destroy();
    
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
    
    // 级别到字符串的映射表
    static std::vector<std::string> _strLevelMap;   
    // 实例数组，每个级别一个实例
    static std::vector<Logger*> _instances;
    // 阻塞队列
    static BlockingQueue<std::string> _queue;            
    
};
#endif