#include "LogSystem.h"

void LogSystem::Init()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("%^[%l][%@]%$ %v");

    spdlog::sinks_init_list sink_list = {console_sink};
    spdlog::init_thread_pool(4096, 1);
    // threadPool = spdlog::thread_pool();
    // defaultLogger = spdlog::default_logger();

    logger = std::make_shared<spdlog::async_logger>("logger", sink_list.begin(), sink_list.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    
    spdlog::register_logger(logger);
}

void LogSystem::Destroy()
{
    logger->flush();
    spdlog::drop_all();
}