#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace Logger
{
    void fatal(const std::string &message, int exitCode=1);
    void error(const std::string &message);
    void warning(const std::string &message);
    void info(const std::string &message);
}

#endif // LOGGER_H
