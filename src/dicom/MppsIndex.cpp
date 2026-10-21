#include "dicomserver/MppsIndex.hpp"
#include "dicomserver/Logging.hpp"

namespace dicomserver {

MppsIndex& MppsIndex::Instance() {
    static MppsIndex idx;
    return idx;
}

bool MppsIndex::Create(const MppsRecord& rec) {
    if (rec.sopInstanceUid.empty()) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (byUid_.count(rec.sopInstanceUid)) return false;
    byUid_[rec.sopInstanceUid] = rec;
    DS_INFO("MPPS CREATE  uid={}  status={}  patient={}  modality={}",
            rec.sopInstanceUid, rec.status, rec.patientId, rec.modality);
    return true;
}

bool MppsIndex::Update(std::string_view sopInstanceUid, const MppsRecord& patch) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = byUid_.find(std::string(sopInstanceUid));
    if (it == byUid_.end()) return false;
    auto& r = it->second;
    auto setIf = [](std::string& dst, const std::string& src) {
        if (!src.empty()) dst = src;
    };
    setIf(r.status, patch.status);
    setIf(r.patientId, patch.patientId);
    setIf(r.patientName, patch.patientName);
    setIf(r.accessionNumber, patch.accessionNumber);
    setIf(r.modality, patch.modality);
    setIf(r.studyInstanceUid, patch.studyInstanceUid);
    setIf(r.performedStationAet, patch.performedStationAet);
    setIf(r.performedProcedureStepStartDate, patch.performedProcedureStepStartDate);
    setIf(r.performedProcedureStepStartTime, patch.performedProcedureStepStartTime);
    setIf(r.performedProcedureStepEndDate, patch.performedProcedureStepEndDate);
    setIf(r.performedProcedureStepEndTime, patch.performedProcedureStepEndTime);
    setIf(r.performedProcedureStepDescription, patch.performedProcedureStepDescription);
    setIf(r.performedProcedureStepId, patch.performedProcedureStepId);
    DS_INFO("MPPS SET  uid={}  status={}", r.sopInstanceUid, r.status);
    return true;
}

std::optional<MppsRecord> MppsIndex::Get(std::string_view sopInstanceUid) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = byUid_.find(std::string(sopInstanceUid));
    if (it == byUid_.end()) return std::nullopt;
    return it->second;
}

std::vector<MppsRecord> MppsIndex::ListAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MppsRecord> out;
    out.reserve(byUid_.size());
    for (const auto& [_, r] : byUid_) out.push_back(r);
    return out;
}

std::size_t MppsIndex::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return byUid_.size();
}

void MppsIndex::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    byUid_.clear();
}

}  // namespace dicomserver
