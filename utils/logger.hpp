#pragma once

#include "../main.hpp"

class Logger : public Singleton<Logger>
{
private:
    std::ofstream m_LogFile;
    std::mutex m_Mutex;

public:
    Logger();
    ~Logger();
    
    void Log(const char *format, ...);
};