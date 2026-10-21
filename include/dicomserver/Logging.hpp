#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <string>
#include <string_view>

namespace dicomserver {

class Logging {
public:
    static void Init(std::string_view level = "info",
                     std::string_view logFile = {});
    static void Shutdown();
    static void SetLevel(std::string_view level);
    static std::shared_ptr<spdlog::logger> Get();

private:
    static spdlog::level::level_enum ParseLevel(std::string_view level);
};

}  // namespace dicomserver

#define DS_TRACE(...)    SPDLOG_LOGGER_TRACE(dicomserver::Logging::Get(), __VA_ARGS__)
#define DS_DEBUG(...)    SPDLOG_LOGGER_DEBUG(dicomserver::Logging::Get(), __VA_ARGS__)
#define DS_INFO(...)     SPDLOG_LOGGER_INFO(dicomserver::Logging::Get(), __VA_ARGS__)
#define DS_WARN(...)     SPDLOG_LOGGER_WARN(dicomserver::Logging::Get(), __VA_ARGS__)
#define DS_ERROR(...)    SPDLOG_LOGGER_ERROR(dicomserver::Logging::Get(), __VA_ARGS__)
#define DS_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(dicomserver::Logging::Get(), __VA_ARGS__)
