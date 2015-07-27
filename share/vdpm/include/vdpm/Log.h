#ifndef VDPM_LOG_H
#define VDPM_LOG_H

namespace vdpm
{
    class Log
    {
    public:
        ~Log();

        static Log& getInstance();
        static void(*println)(const char *format, ...);
        
    private:
        Log();
        
        static void printlnEmpty(const char *format, ...);
    };
} // namespace vdpm

#endif // VDPM_LOG_H
