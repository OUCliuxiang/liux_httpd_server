#include "logger.h"

#include <assert.h>
#include <string>
#include <pthread.h> // 异步写日志，单独开一个线程

using namespace std;

char Logger::seperator = '|';
int Logger::_curruct_level = TRACE;
ostream& Logger::_output(cerr); // 初始化到标准错误 

vector<string> Logger::_strLevelMap = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"};
vector<Logger*> Logger::_instances(LEVEL_SIZE, nullptr);
BlockingQueue<string> Logger::_queue;

Logger* Logger::instance(int level) {
    assert(level > 0 && level < LEVEL_SIZE);
    if (Logger::_instances[level] == nullptr) 
        Logger::_instances[level] = new Logger(level);
    return Logger::_instances[level];
}

void Logger::destroy() {
    for (Logger* p: _instances) 
        delete p;
}

// 这个函数将会单独开一个线程运行，只要阻塞队列里有数据，就立刻输出
void Logger::thread_func() {
    while(true) {
        const string& msg = _queue.front();
        _output << msg << endl;
        _queue.pop_front();
    }
}

void Logger::append(const string& log) {
    _queue.push_back(log);
}

const std::string& Logger::strLevel() {
    return _strLevelMap[_level];
}

void Logger::logging(const string& file, int line, const string& content) {
    if (_level >= _curruct_level) {
        std::string msg;

        msg.append(Helper::date());
        msg.push_back(seperator);

        msg.append(Helper::time());
        msg.push_back(seperator);

        msg.append(std::to_string(pthread_self())); // 获取 pthread 线程 pid
        msg.push_back(seperator);

        msg.append(strLevel());
        msg.push_back(seperator);

        msg.append(content);
        msg.push_back(seperator);

        msg.append(file);
        msg.push_back(':');

        msg.append(std::to_string(line));
        msg.push_back(seperator);

        _queue.push_back(msg);
    }
}
