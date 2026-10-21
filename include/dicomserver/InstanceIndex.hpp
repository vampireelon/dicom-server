#pragma once

#include "Config.hpp"
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace dicomserver {

struct InstanceMeta {
    std::string sopInstanceUid;
    std::string sopClassUid;
    std::string studyInstanceUid;
    std::string seriesInstanceUid;
    std::string patientId;
    std::string patientName;
    std::string studyDate;
    std::string modality;
    std::string accessionNumber;
    std::string filePath;
    int64_t fileSize{0};
    std::string receivedAt;
    nlohmann::json extra;
};

struct StudySummary {
    std::string studyInstanceUid;
    std::string patientId;
    std::string patientName;
    std::string studyDate;
    std::string accessionNumber;
    std::string modalities;
    int seriesCount{0};
    int instanceCount{0};
};

class InstanceIndex {
public:
    explicit InstanceIndex(const Config& config);
    ~InstanceIndex();

    InstanceIndex(const InstanceIndex&) = delete;
    InstanceIndex& operator=(const InstanceIndex&) = delete;

    bool Open();
    void Close();

    // C-STORE path
    bool Insert(const InstanceMeta& meta);
    bool Exists(const std::string& sopInstanceUid) const;

    // Query helpers (C-FIND / QIDO)
    std::vector<InstanceMeta> FindInstances(const std::map<std::string, std::string>& query) const;
    std::vector<StudySummary> FindStudies(const std::map<std::string, std::string>& query) const;
    std::optional<InstanceMeta> GetBySop(const std::string& sopInstanceUid) const;
    std::vector<InstanceMeta> GetByStudy(const std::string& studyUid) const;
    std::vector<InstanceMeta> GetBySeries(const std::string& seriesUid) const;

    // Storage path resolution
    std::string ResolveStoragePath(const std::string& studyUid, const std::string& seriesUid,
                                   const std::string& sopUid) const;

    // Stats
    size_t CountInstances() const;
    size_t CountStudies() const;

    // Backend type
    enum class Backend { SQLite, PostgreSQL };
    Backend GetBackend() const { return backend_; }

private:
    void InitSqlite();
    void InitPostgres();
    void EnsureSchema();

    Config config_;
    Backend backend_{Backend::SQLite};
    mutable std::mutex mutex_;

    // Opaque handles (sqlite3* or PGconn*)
    void* db_{nullptr};
};

}  // namespace dicomserver
