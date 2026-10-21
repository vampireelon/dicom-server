#pragma once

#include "dicomserver/Exception.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dicomserver {

struct ServerConfig {
    std::string aet{"DICOMSERVER"};
    uint16_t    port{4242};
    bool        checkCalledAet{true};
    uint32_t    associationTimeoutSec{30};
    unsigned    workerThreads{4};

    std::string logLevel{"info"};
    std::string logFile{};

    std::string storageDirectory{"./storage"};
    std::string worklistDirectory{"./worklist"};

    std::string databaseBackend{"sqlite"};
    std::string databasePath{"./dicomserver.db"};
    std::string postgresConnInfo{};
    bool        rebuildIndexOnStart{false};

    std::string pluginDirectory{"./plugins"};

    bool        aclEnabled{false};
    std::string aclDefaultPolicy{"allow"};
    struct AclRule {
        std::string callingAet;
        std::vector<std::string> allow;
    };
    std::vector<AclRule> aclRules;
    unsigned associationRateLimitPerMinute{0};
    bool        anonymizeHttpExports{false};
    bool        auditHashChain{false};
    std::string auditLogFile{"./audit.log"};

    bool        httpEnabled{true};
    std::string httpBind{"0.0.0.0"};
    uint16_t    httpPort{8042};
    std::string httpApiToken{};

    unsigned maxAssociations{64};
    unsigned maxDiskUsagePercent{95};
    unsigned auditMaxSizeMb{100};

    bool        dicomTls{false};
    std::string tlsCertificateFile{};
    std::string tlsPrivateKeyFile{};
    std::string tlsTrustedCertFile{};
    bool        tlsRequireClientCert{false};

    struct RemoteModality {
        std::string aet;
        std::string host;
        uint16_t    port{104};
    };
    std::vector<RemoteModality> modalities;

    nlohmann::json raw;
};

class Config {
public:
    static void Load(std::string_view path);
    static void LoadFromJson(const nlohmann::json& j);
    static bool Reload();
    static const ServerConfig& Get();
    static std::string ConfigPath();
    static void Reset();

private:
    static ServerConfig Parse(const nlohmann::json& j);
    static void Validate(const ServerConfig& cfg);
    static std::optional<ServerConfig> instance_;
    static std::string configPath_;
};

}  // namespace dicomserver
