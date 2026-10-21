#include "dicomserver/Config.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace dicomserver {

namespace {

std::string GetString(const nlohmann::json& j, const std::string& key, const std::string& def = "") {
    if (j.contains(key) && j[key].is_string()) return j[key].get<std::string>();
    return def;
}

int GetInt(const nlohmann::json& j, const std::string& key, int def = 0) {
    if (j.contains(key) && j[key].is_number_integer()) return j[key].get<int>();
    if (j.contains(key) && j[key].is_number()) return static_cast<int>(j[key].get<double>());
    return def;
}

bool GetBool(const nlohmann::json& j, const std::string& key, bool def = false) {
    if (j.contains(key) && j[key].is_boolean()) return j[key].get<bool>();
    return def;
}

}  // namespace

Config Config::LoadFromFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("Cannot open config file: " + path);
    }
    nlohmann::json j;
    ifs >> j;
    return Config::FromJson(j);
}

Config Config::FromJson(const nlohmann::json& j) {
    Config c;

    c.aeTitle = GetString(j, "aeTitle", "DICOM_SERVER");
    c.dicomPort = GetInt(j, "dicomPort", 11112);
    c.httpPort = GetInt(j, "httpPort", 8080);
    c.bindAddress = GetString(j, "bindAddress", "0.0.0.0");
    c.storagePath = GetString(j, "storagePath", "./storage");
    c.maxAssociations = GetInt(j, "maxAssociations", 32);
    c.idleTimeoutSec = GetInt(j, "idleTimeoutSec", 60);

    if (j.contains("index") && j["index"].is_object()) {
        const auto& idx = j["index"];
        c.indexBackend = GetString(idx, "backend", "sqlite");
        c.sqlitePath = GetString(idx, "sqlitePath", "./index.db");
        c.pgHost = GetString(idx, "pgHost", "localhost");
        c.pgPort = GetInt(idx, "pgPort", 5432);
        c.pgDatabase = GetString(idx, "pgDatabase", "dicom");
        c.pgUser = GetString(idx, "pgUser", "dicom");
        c.pgPassword = GetString(idx, "pgPassword", "");
    }

    if (j.contains("acl") && j["acl"].is_object()) {
        const auto& acl = j["acl"];
        c.aclEnabled = GetBool(acl, "enabled", true);
        if (acl.contains("allowedAeTitles") && acl["allowedAeTitles"].is_array()) {
            for (const auto& a : acl["allowedAeTitles"]) {
                if (a.is_string()) c.allowedAeTitles.push_back(a.get<std::string>());
            }
        }
        c.rateLimitPerMinute = GetInt(acl, "rateLimitPerMinute", 120);
    }

    if (j.contains("plugins") && j["plugins"].is_object()) {
        const auto& p = j["plugins"];
        c.pluginsEnabled = GetBool(p, "enabled", false);
        c.pluginsDir = GetString(p, "directory", "./plugins");
    }

    if (j.contains("tls") && j["tls"].is_object()) {
        const auto& t = j["tls"];
        c.tlsEnabled = GetBool(t, "enabled", false);
        c.tlsCertFile = GetString(t, "certFile", "");
        c.tlsKeyFile = GetString(t, "keyFile", "");
    }

    if (j.contains("logging") && j["logging"].is_object()) {
        const auto& l = j["logging"];
        c.logLevel = GetString(l, "level", "info");
        c.logFile = GetString(l, "file", "");
    }

    if (j.contains("audit") && j["audit"].is_object()) {
        c.auditEnabled = GetBool(j["audit"], "enabled", true);
        c.auditPath = GetString(j["audit"], "path", "./audit.log");
    }

    return c;
}

nlohmann::json Config::ToJson() const {
    nlohmann::json j;
    j["aeTitle"] = aeTitle;
    j["dicomPort"] = dicomPort;
    j["httpPort"] = httpPort;
    j["bindAddress"] = bindAddress;
    j["storagePath"] = storagePath;
    j["maxAssociations"] = maxAssociations;
    j["idleTimeoutSec"] = idleTimeoutSec;

    j["index"] = {
        {"backend", indexBackend},
        {"sqlitePath", sqlitePath},
        {"pgHost", pgHost},
        {"pgPort", pgPort},
        {"pgDatabase", pgDatabase},
        {"pgUser", pgUser},
        {"pgPassword", pgPassword}
    };

    j["acl"] = {
        {"enabled", aclEnabled},
        {"allowedAeTitles", allowedAeTitles},
        {"rateLimitPerMinute", rateLimitPerMinute}
    };

    j["plugins"] = {
        {"enabled", pluginsEnabled},
        {"directory", pluginsDir}
    };

    j["tls"] = {
        {"enabled", tlsEnabled},
        {"certFile", tlsCertFile},
        {"keyFile", tlsKeyFile}
    };

    j["logging"] = {
        {"level", logLevel},
        {"file", logFile}
    };

    j["audit"] = {
        {"enabled", auditEnabled},
        {"path", auditPath}
    };

    return j;
}

void Config::SaveToFile(const std::string& path) const {
    std::ofstream ofs(path);
    if (!ofs) throw std::runtime_error("Cannot write config: " + path);
    ofs << ToJson().dump(2);
}

}  // namespace dicomserver
