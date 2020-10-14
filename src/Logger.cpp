#include <iostream>
#include <iomanip>
#include <chrono>

#include "Logger.h"

#define LOG_NO_COLOR

static inline std::string getTime()
{
    namespace chr = std::chrono;

    auto timeInMicroseconds{chr::duration_cast<chr::microseconds>(chr::system_clock::now().time_since_epoch()).count()};
    std::time_t timeInSeconds{timeInMicroseconds/1000000};
    std::tm tstruct{*localtime(&timeInSeconds)};
    char buffer[9]{};

    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tstruct);

    return std::string(buffer)+"."+std::to_string(timeInMicroseconds%1000000);
}

void Logger::fatal(const std::string &message, int exitCode)
{
    using namespace std::chrono;

    std::cerr << std::dec;

    #ifndef LOG_NO_COLOR
        std::cerr << "\033[30;41m[" << getTime() << "][Fatal]" << "\033[0;0m: " << message << '\n';
    #else
        std::cerr << "[" << getTime() << "][Fatal]: " << message << '\n';
    #endif
    std::cerr.flush();
    
    std::exit(exitCode);
}

void Logger::error(const std::string &message)
{
    using namespace std::chrono;

    std::cerr << std::dec;

    #ifndef LOG_NO_COLOR
        std::cerr << "\033[31m[" << getTime() << "][Error]" << "\033[0;0m: " << message << '\n';
    #else
        std::cerr << "[" << getTime() << "][Error]: " << message << '\n';
    #endif
    
    std::cerr.flush();
}

void Logger::warning(const std::string &message)
{
    using namespace std::chrono;

    std::cerr << std::dec;

    #ifndef LOG_NO_COLOR
        std::cerr << "\033[33m[" << getTime() << "][Warning]" << "\033[0;0m: " << message << '\n';
    #else
        std::cerr << "[" << getTime() << "][Warning]: " << message << '\n';
    #endif

    std::cerr.flush();
}

void Logger::info(const std::string &message)
{
    using namespace std::chrono;

    std::cout << std::dec;

    #ifndef LOG_NO_COLOR
        std::cout << "\033[32m[" << getTime() << "][Info]" << "\033[0;0m: " << message << '\n';
    #else
        std::cout << "[" << getTime() <<"][Info]: " << message << '\n';
    #endif
    
    std::cout.flush();
}
