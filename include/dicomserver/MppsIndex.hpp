#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dicomserver {

struct MppsRecord {
    std::string sopInstanceUid;
    std::string status{"IN PROGRESS"};
    std::string patientId;
    std::string patientName;
    std::string accessionNumber;
    std::string modality;
    std::string studyInstanceUid;
    std::string performedStationAet;
    std::string performedProcedureStepStartDate;
    std::string performedProcedureStepStartTime;
    std::string performedProcedureStepEndDate;
    std::string performedProcedureStepEndTime;
    std::string performedProcedureStepDescription;
    std::string performedProcedureStepId;
    std::string callingAet;
};

class MppsIndex {
public:
    static MppsIndex& Instance();
    bool Create(const MppsRecord& rec);
    bool Update(std::string_view sopInstanceUid, const MppsRecord& patch);
    [[nodiscard]] std::optional<MppsRecord> Get(std::string_view sopInstanceUid) const;
    [[nodiscard]] std::vector<MppsRecord> ListAll() const;
    [[nodiscard]] std::size_t Size() const;
    void Clear();

private:
    MppsIndex() = default;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, MppsRecord> byUid_;
};

}  // namespace dicomserver
