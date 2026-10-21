#include "dicomserver/Config.hpp"
#include "dicomserver/DicomServer.hpp"
#include "dicomserver/HttpServer.hpp"
#include "dicomserver/Exception.hpp"
#include "dicomserver/Logging.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <ctime>
#endif

namespace {

volatile std::sig_atomic_t g_stopRequested = 0;
volatile std::sig_atomic_t g_reloadRequested = 0;

void SignalHandler(int signum) {
    if (signum == SIGHUP) {
        g_reloadRequested = 1;
        return;
    }
    g_stopRequested = 1;
}

void PrintUsage(const char* argv0) {
    std::cerr
        << "Usage: " << argv0 << " [--config <path>] [--help]\n"
        << "\n"
        << "  --config <path>   Path to JSON configuration file\n"
        << "                    (default: config/server.json)\n"
        << "  --help            Show this help\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string configPath = "config/server.json";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            return 0;
        }
        if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
            continue;
        }
        std::cerr << "Unknown argument: " << arg << "\n";
        PrintUsage(argv[0]);
        return 1;
    }

    try {
        dicomserver::Config::Load(configPath);
        const auto& cfg = dicomserver::Config::Get();

        dicomserver::Logging::Init(cfg.logLevel, cfg.logFile);

        DS_INFO("=================================================");
        DS_INFO("  DICOM Server  v1.8.0  (Phase 14 – DICOMweb)");
        DS_INFO("=================================================");
        DS_INFO("Config file        : {}", configPath);
        DS_INFO("AE Title           : {}", cfg.aet);
        DS_INFO("Listen port        : {}", cfg.port);
        DS_INFO("Worker threads     : {}", cfg.workerThreads);

        dicomserver::DicomServer server;
        server.Start(cfg);

        dicomserver::HttpServer http;
        http.Start(cfg);

        std::signal(SIGINT, SignalHandler);
        std::signal(SIGTERM, SignalHandler);
#if !defined(_WIN32)
        std::signal(SIGHUP, SignalHandler);
#endif

        DS_INFO("Ready. DICOM: echoscu -aec {} localhost {}", cfg.aet, cfg.port);
        if (cfg.httpEnabled)
            DS_INFO("Ready. HTTP : curl http://localhost:{}/health", cfg.httpPort);

        while (!g_stopRequested) {
            if (g_reloadRequested) {
                g_reloadRequested = 0;
                DS_INFO("SIGHUP received – reloading config...");
                if (dicomserver::Config::Reload()) {
                    server.ReloadHotConfig();
                } else {
                    DS_ERROR("Config reload failed – keeping previous settings");
                }
            }
#if defined(_WIN32)
            Sleep(200);
#else
            struct timespec ts = {0, 200 * 1000 * 1000};
            nanosleep(&ts, nullptr);
#endif
        }

        DS_INFO("Shutdown requested...");
        http.Stop();
        server.Stop();
        dicomserver::Logging::Shutdown();
        return 0;

    } catch (const dicomserver::Exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return static_cast<int>(e.Code());
    } catch (const std::exception& e) {
        std::cerr << "Fatal (std): " << e.what() << "\n";
        return 1;
    }
}
