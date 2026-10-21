#pragma once

#include "Config.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace dicomserver {

struct InstanceRecord {
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
};

struct StudySummary {
    std::string studyInstanceUid;
    std::string patientId;
    std::string patientName;
    std::string studyDate;
    std::string accessionNumber;
    std::string modalitiesInStudy;
    int numberOfSeries{0};
    int numberOfInstances{0};
};

class InstanceIndex {
public:
    InstanceIndex();
    ~InstanceIndex();

    InstanceIndex(const InstanceIndex&) = delete;
    InstanceIndex& operator=(const InstanceIndex&) = delete;

    void Open();
    void Close();

    bool Insert(const InstanceRecord& rec);
    bool Exists(const std::string& sopInstanceUid) const;

    std::vector<InstanceRecord> Find(const std::map<std::string, std::string>& query) const;
    std::vector<StudySummary> FindStudies(const std::map<std::string, std::string>& query) const;
    std::optional<InstanceRecord> Get(const std::string& sopInstanceUid) const;
    std::vector<InstanceRecord> ListByStudy(const std::string& studyUid) const;
    std::vector<InstanceRecord> ListBySeries(const std::string& seriesUid) const;

    size_t Count() const;

private:
    void InitSchema();
    void* db_{nullptr};  // sqlite3* or PGconn*
    bool usePostgres_{false};
    mutable std::mutex mu_;
};

}  // namespace dicomserver
