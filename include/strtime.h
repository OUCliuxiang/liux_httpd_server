#ifndef __STRTIME_H__
#define __STRTIME_H__

#include <string>
#include <time.h>
#include <memory>

class StrTime {
public:
    using ptr = std::shared_ptr<StrTime>;
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