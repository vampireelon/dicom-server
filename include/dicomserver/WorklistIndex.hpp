#pragma once

#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dicomserver {

struct WorklistItem {
    std::string accessionNumber;
    std::string patientId;
    std::string patientName;
    std::string patientBirthDate;
    std::string patientSex;
    std::string modality;
    std::string scheduledStationAet;
    std::string scheduledProcedureStepStartDate;
    std::string scheduledProcedureStepStartTime;
    std::string scheduledPerformingPhysician;
    std::string scheduledProcedureStepDescription;
    std::string scheduledProcedureStepId;
    std::string requestedProcedureId;
    std::string studyInstanceUid;
    std::string referringPhysicianName;
};

class WorklistIndex {
public:
    static WorklistIndex& Instance();
    void LoadFromDirectory(std::string_view dir);
    void Add(const WorklistItem& item);
    void Clear();
    [[nodiscard]] std::size_t Size() const;
    [[nodiscard]] std::vector<WorklistItem> Find(
        const std::unordered_map<std::string, std::string>& keys) const;

private:
    WorklistIndex() = default;
    static bool Matches(std::string_view value, std::string_view pattern);
    mutable std::mutex mutex_;
    std::vector<WorklistItem> items_;
};

}  // namespace dicomserver
