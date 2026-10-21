#include "dicomserver/Config.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace dicomserver {

std::optional<ServerConfig> Config::instance_;
std::string Config::configPath_;

namespace {

template <typename T>
T GetOr(const nlohmann::json& j, std::string_view key, T defaultValue) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return defaultValue;
}

}  // namespace

ServerConfig Config::Parse(const nlohmann::json& j) {
    ServerConfig cfg;
    cfg.raw = j;

    cfg.aet                   = GetOr<std::string>(j, "DicomAet", cfg.aet);
    cfg.port                  = static_cast<uint16_t>(GetOr<int>(j, "DicomPort", cfg.port));
    cfg.checkCalledAet        = GetOr<bool>(j, "DicomCheckCalledAet", cfg.checkCalledAet);
    cfg.associationTimeoutSec = static_cast<uint32_t>(
        GetOr<int>(j, "DicomAssociationTimeout", static_cast<int>(cfg.associationTimeoutSec)));
    cfg.workerThreads         = static_cast<unsigned>(
        GetOr<int>(j, "DicomThreads", static_cast<int>(cfg.workerThreads)));

    cfg.logLevel = GetOr<std::string>(j, "LogLevel", cfg.logLevel);
    cfg.logFile  = GetOr<std::string>(j, "LogFile", cfg.logFile);

    cfg.storageDirectory = GetOr<std::string>(j, "StorageDirectory", cfg.storageDirectory);

    cfg.worklistDirectory = GetOr<std::string>(j, "WorklistDirectory", cfg.worklistDirectory);

    cfg.databaseBackend = GetOr<std::string>(j, "DatabaseBackend", cfg.databaseBackend);
    cfg.databasePath = GetOr<std::string>(j, "DatabasePath", cfg.databasePath);
    cfg.postgresConnInfo = GetOr<std::string>(j, "PostgresConnInfo", cfg.postgresConnInfo);
    cfg.rebuildIndexOnStart = GetOr<bool>(j, "RebuildIndexOnStart", cfg.rebuildIndexOnStart);
    cfg.pluginDirectory = GetOr<std::string>(j, "PluginDirectory", cfg.pluginDirectory);

    cfg.aclEnabled = GetOr<bool>(j, "AccessControlEnabled", cfg.aclEnabled);
    cfg.aclDefaultPolicy = GetOr<std::string>(j, "AccessControlDefaultPolicy", cfg.aclDefaultPolicy);
    cfg.associationRateLimitPerMinute = static_cast<unsigned>(
        GetOr<int>(j, "AssociationRateLimitPerMinute", static_cast<int>(cfg.associationRateLimitPerMinute)));
    cfg.anonymizeHttpExports = GetOr<bool>(j, "AnonymizeHttpExports", cfg.anonymizeHttpExports);
    cfg.auditHashChain = GetOr<bool>(j, "AuditHashChain", cfg.auditHashChain);

    if (j.contains("AccessControlRules") && j["AccessControlRules"].is_array()) {
        for (const auto& item : j["AccessControlRules"]) {
            if (!item.is_object()) continue;
            ServerConfig::AclRule rule;
            rule.callingAet = GetOr<std::string>(item, "CallingAet", "");
            if (item.contains("Allow") && item["Allow"].is_array()) {
                for (const auto& op : item["Allow"]) {
                    if (op.is_string()) rule.allow.push_back(op.get<std::string>());
                }
            }
            if (!rule.callingAet.empty()) cfg.aclRules.push_back(std::move(rule));
        }
    }

    cfg.auditLogFile = GetOr<std::string>(j, "AuditLogFile", cfg.auditLogFile);

    cfg.httpEnabled = GetOr<bool>(j, "HttpEnabled", cfg.httpEnabled);
    cfg.httpBind = GetOr<std::string>(j, "HttpBindAddress", cfg.httpBind);
    cfg.httpPort = static_cast<uint16_t>(GetOr<int>(j, "HttpPort", static_cast<int>(cfg.httpPort)));
    cfg.httpApiToken = GetOr<std::string>(j, "HttpApiToken", cfg.httpApiToken);

    cfg.maxAssociations = static_cast<unsigned>(GetOr<int>(j, "MaxAssociations", static_cast<int>(cfg.maxAssociations)));
    cfg.maxDiskUsagePercent = static_cast<unsigned>(GetOr<int>(j, "MaxDiskUsagePercent", static_cast<int>(cfg.maxDiskUsagePercent)));
    cfg.auditMaxSizeMb = static_cast<unsigned>(GetOr<int>(j, "AuditMaxSizeMb", static_cast<int>(cfg.auditMaxSizeMb)));

    cfg.dicomTls = GetOr<bool>(j, "DicomTlsEnabled", cfg.dicomTls);
    cfg.tlsCertificateFile = GetOr<std::string>(j, "DicomTlsCertificateFile", cfg.tlsCertificateFile);
    cfg.tlsPrivateKeyFile = GetOr<std::string>(j, "DicomTlsPrivateKeyFile", cfg.tlsPrivateKeyFile);
    cfg.tlsTrustedCertFile = GetOr<std::string>(j, "DicomTlsTrustedCertificateFile", cfg.tlsTrustedCertFile);
    cfg.tlsRequireClientCert = GetOr<bool>(j, "DicomTlsRemoteCertificateRequired", cfg.tlsRequireClientCert);

    // Modalities: { "AET": ["AET", "host", port], ... }
    if (j.contains("DicomModalities") && j["DicomModalities"].is_object()) {
        for (auto it = j["DicomModalities"].begin(); it != j["DicomModalities"].end(); ++it) {
            const auto& arr = it.value();
            if (!arr.is_array() || arr.size() < 2) {
                throw Exception(ErrorCode::ConfigInvalidValue,
                                "DicomModalities entry '" + it.key() +
                                    "' must be an array [AET, host, port?]");
            }
            ServerConfig::RemoteModality m;
            m.aet  = arr[0].get<std::string>();
            m.host = arr[1].get<std::string>();
            m.port = arr.size() >= 3 ? static_cast<uint16_t>(arr[2].get<int>()) : 104;
            cfg.modalities[it.key()] = std::move(m);
        }
    }

    return cfg;
}

void Config::Load(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw Exception(ErrorCode::ConfigFileNotFound, "Cannot open config: " + path);
    }
    nlohmann::json j;
    try {
        ifs >> j;
    } catch (const nlohmann::json::exception& e) {
        throw Exception(ErrorCode::ConfigParseError, std::string("JSON parse error: ") + e.what());
    }
    instance_ = Parse(j);
    configPath_ = path;
}

void Config::Reload() {
    if (configPath_.empty()) {
        throw Exception(ErrorCode::ConfigNotLoaded, "No config path for reload");
    }
    Load(configPath_);
}

const ServerConfig& Config::Get() {
    if (!instance_) {
        throw Exception(ErrorCode::ConfigNotLoaded, "Config not loaded");
    }
    return *instance_;
}

bool Config::IsLoaded() {
    return instance_.has_value();
}

std::string Config::Path() {
    return configPath_;
}

}  // namespace dicomserver
