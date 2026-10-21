#pragma once

#include "dicomserver/Config.hpp"

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

namespace dicomserver {

class HttpServer {
public:
    HttpServer();
    ~HttpServer();
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    void Start(const ServerConfig& config);
    void Stop();
    [[nodiscard]] bool IsRunning() const noexcept { return running_.load(); }
    [[nodiscard]] uint16_t GetPort() const noexcept { return port_; }

private:
    void AcceptLoop();
    void HandleSession(int socketFd);
    std::string bindAddr_{"0.0.0.0"};
    uint16_t    port_{8042};
    std::string apiToken_;
    std::atomic<bool> running_{false};
    int               listenFd_{-1};
    std::thread       thread_;
};

}  // namespace dicomserver
