#ifndef __STRTIME_H__
#define __STRTIME_H__

#include <string>
#include <time.h>

class StrTime {
public:
    StrTime() {
        ::time(&rawtime);
        timeinfo = localtime(&rawtime);
    }
    
    std::string to_string(const char* format) {
        char buf[64];
        strftime(buf, sizeof(buf), format, timeinfo);
        return buf;
    }
    
private:
    time_t rawtime;
    struct tm* timeinfo;
};

#endif