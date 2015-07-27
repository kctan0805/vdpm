#include "vdpm/Log.h"

using namespace vdpm;

void(*Log::println)(const char *format, ...) = Log::printlnEmpty;

Log::Log()
{
    // do nothing
}

Log::~Log()
{
    // do nothing
}

Log& Log::getInstance()
{
    static Log self;
    return self;
}

void Log::printlnEmpty(const char *format, ...)
{
    // do nothing
}
