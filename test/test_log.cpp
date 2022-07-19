#include "logger.h"

int main() {
    Logger* log = Logger::instance(2);
    delete log;
    
    return 0;
}