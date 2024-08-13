#pragma once

#include <memory>
#include <string>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE  // LOG级别
#include "SPDLog.h"

class LogSystem 
{
public:
    LogSystem() = default;
    ~LogSystem() {};

    void Init();
    void Destroy();

    void Debug(std::string message)  { logger->debug(message); }
    void Info(std::string message)   { logger->info(message); }
    void Warn(std::string message)   { logger->warn(message); }
    void Fatal(std::string message)  { logger->critical(message); throw std::runtime_error(message); }

    std::shared_ptr<spdlog::logger> GetLogger() { return logger; };

private:
    std::shared_ptr<spdlog::logger> logger;
};

