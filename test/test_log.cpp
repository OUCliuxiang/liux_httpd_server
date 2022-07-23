#include "logger.h"
#include <sys/time.h>
#include <thread>
#include <unistd.h>

using namespace std;

void write_logging()
{
    thread log_thread(Logger::thread_func);
    log_thread.detach();
}

void trace()
{
    while (1)   {
        TRACE("trace_test");
        sleep(1);
    }
}
void debug()
{
    while (1)   {
        DEBUG("debug_test");
        sleep(1);
    }
}
void info()
{
    while (1)   {
        INFO("info_test");
        sleep(1);
    }
}
void warn()
{
    while (1)   {
        WARN("warn_test");
        sleep(1);
    }
}
void error()
{
    while (1)   {
        ERROR("error_test");
        sleep(1);
    }
}


void trace_test()
{
    thread t(trace);
    t.detach();
}

void debug_test()
{
    thread t(debug);
    t.detach();
}

void info_test()
{
    thread t(info);
    t.detach();
}

void warn_test()
{
    thread t(warn);
    t.detach();
}

void error_test()
{
    thread t(error);
    t.detach();
}

void test()
{
    write_logging();
    
    trace_test();
    debug_test();
    info_test();
    warn_test();
    error_test();
}

int main(int ac, char** av)
{
    test();
    
    while (1)   sleep(1000);
    return 0;
}