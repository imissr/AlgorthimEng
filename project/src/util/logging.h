#ifndef ALGENG_UTIL_LOGGING_H
#define ALGENG_UTIL_LOGGING_H

#include <string>

enum class LogLevel { Error=0, Info=1, Debug=2 };

void setLogLevel(LogLevel lvl);
LogLevel getLogLevel();

void logInfo(const std::string& msg);
void logDebug(const std::string& msg);
void logError(const std::string& msg);

#endif
