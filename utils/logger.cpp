#include "logger.hpp"

Logger::Logger(){
    m_LogFile.open("log.txt", std::ios::app);
}

Logger::~Logger(){
    if(m_LogFile.is_open())
        m_LogFile.close();
}

void Logger::Log(const char *format, ...)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    va_list ap;
    va_start(ap, format);

    char buffer[2048];
    std::vsnprintf(buffer, sizeof(buffer), format, ap);

    va_end(ap);

#ifdef _WIN32
    char output[2048];
    CharToOem(buffer, output);
    std::cout << output << std::endl;
#else
    std::cout << buffer << std::endl;
#endif

    if (m_LogFile.is_open()) {
        time_t now = std::time(nullptr);
        tm* tm = std::localtime(&now);
        char timeBuffer[256];
        std::strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S]", tm);

        m_LogFile << timeBuffer << " " << buffer << std::endl;
        m_LogFile.flush();
    }
}