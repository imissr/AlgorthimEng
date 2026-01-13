#include "logging.h"
#include <iostream>

static LogLevel g_level = LogLevel::Info;

void setLogLevel(LogLevel lvl) { g_level = lvl; }
LogLevel getLogLevel() { return g_level; }

void logInfo(const std::string& msg) {
    if ((int)g_level >= (int)LogLevel::Info) std::cout << "[info] " << msg << "\n";
}
void logDebug(const std::string& msg) {
    if ((int)g_level >= (int)LogLevel::Debug) std::cout << "[debug] " << msg << "\n";
}
void logError(const std::string& msg) {
    std::cerr << "[error] " << msg << "\n";
}
