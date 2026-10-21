#include "dicomserver/Logging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <vector>

namespace dicomserver {

namespace {
std::shared_ptr<spdlog::logger> g_logger;
}

spdlog::level::level_enum Logging::ParseLevel(std::string_view level) {
    if (level == "trace")    return spdlog::level::trace;
    if (level == "debug")    return spdlog::level::debug;
    if (level == "info")     return spdlog::level::info;
    if (level == "warn")     return spdlog::level::warn;
    if (level == "error")    return spdlog::level::err;
    if (level == "critical") return spdlog::level::critical;
    return spdlog::level::info;
}

void Logging::Init(std::string_view level, std::string_view logFile) {
    if (g_logger) {
        g_logger->set_level(ParseLevel(level));
        return;
    }

    std::vector<spdlog::sink_ptr> sinks;

    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    sinks.push_back(console);

    if (!logFile.empty()) {
        auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            std::string(logFile), /*truncate=*/false);
        file->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        sinks.push_back(file);
    }

    g_logger = std::make_shared<spdlog::logger>("dicomserver", sinks.begin(), sinks.end());
    g_logger->set_level(ParseLevel(level));
    g_logger->flush_on(spdlog::level::warn);

    spdlog::register_logger(g_logger);
    spdlog::set_default_logger(g_logger);
}

void Logging::Shutdown() {
    if (g_logger) {
        g_logger->flush();
        spdlog::drop("dicomserver");
        g_logger.reset();
    }
    spdlog::shutdown();
}

void Logging::SetLevel(std::string_view level) {
    if (g_logger) {
        g_logger->set_level(ParseLevel(level));
    }
}

std::shared_ptr<spdlog::logger> Logging::Get() {
    if (!g_logger) {
        Init("info");
    }
    return g_logger;
}

}  // namespace dicomserver
