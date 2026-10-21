#include "dicomserver/WorklistIndex.hpp"
#include "dicomserver/Logging.hpp"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcsequen.h>

#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace dicomserver {

namespace {

std::string GetStr(DcmItem* item, const DcmTagKey& tag) {
    if (!item) return {};
    OFString v;
    if (item->findAndGetOFString(tag, v).good()) return v.c_str();
    return {};
}

WorklistItem ParseWorklistDataset(DcmDataset* ds) {
    WorklistItem w;
    if (!ds) return w;
    w.accessionNumber = GetStr(ds, DCM_AccessionNumber);
    w.patientId = GetStr(ds, DCM_PatientID);
    w.patientName = GetStr(ds, DCM_PatientName);
    w.patientBirthDate = GetStr(ds, DCM_PatientBirthDate);
    w.patientSex = GetStr(ds, DCM_PatientSex);
    w.referringPhysicianName = GetStr(ds, DCM_ReferringPhysicianName);
    w.studyInstanceUid = GetStr(ds, DCM_StudyInstanceUID);
    w.requestedProcedureId = GetStr(ds, DCM_RequestedProcedureID);

    DcmSequenceOfItems* spsSeq = nullptr;
    if (ds->findAndGetSequence(DCM_ScheduledProcedureStepSequence, spsSeq).good() &&
        spsSeq && spsSeq->card() > 0) {
        DcmItem* sps = spsSeq->getItem(0);
        w.modality = GetStr(sps, DCM_Modality);
        w.scheduledStationAet = GetStr(sps, DCM_ScheduledStationAETitle);
        w.scheduledProcedureStepStartDate = GetStr(sps, DCM_ScheduledProcedureStepStartDate);
        w.scheduledProcedureStepStartTime = GetStr(sps, DCM_ScheduledProcedureStepStartTime);
        w.scheduledPerformingPhysician = GetStr(sps, DCM_ScheduledPerformingPhysicianName);
        w.scheduledProcedureStepDescription = GetStr(sps, DCM_ScheduledProcedureStepDescription);
        w.scheduledProcedureStepId = GetStr(sps, DCM_ScheduledProcedureStepID);
    }
    return w;
}

}  // namespace

WorklistIndex& WorklistIndex::Instance() {
    static WorklistIndex idx;
    return idx;
}

void WorklistIndex::LoadFromDirectory(std::string_view dir) {
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        DS_WARN("Worklist directory '{}' not found – starting empty", dir);
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    items_.clear();
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (ext != ".wl" && ext != ".dcm" && ext != ".dicom") continue;

        DcmFileFormat ff;
        if (ff.loadFile(entry.path().c_str()).bad()) {
            DS_WARN("Cannot load worklist file {}", entry.path().string());
            continue;
        }
        items_.push_back(ParseWorklistDataset(ff.getDataset()));
        DS_INFO("Worklist loaded: PatientID={}  Accession={}  from {}",
                items_.back().patientId, items_.back().accessionNumber,
                entry.path().filename().string());
    }
    DS_INFO("Worklist index size={}", items_.size());
}

void WorklistIndex::Add(const WorklistItem& item) {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.push_back(item);
}

void WorklistIndex::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.clear();
}

std::size_t WorklistIndex::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return items_.size();
}

bool WorklistIndex::Matches(std::string_view value, std::string_view pattern) {
    if (pattern.empty()) return true;
    if (pattern == "*") return true;

    std::string val(value), pat(pattern);
    auto lower = [](std::string& s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };
    lower(val);
    lower(pat);

    if (pat.find('*') == std::string::npos) return val == pat;

    size_t first = pat.find('*');
    if (first == pat.size() - 1) {
        std::string prefix = pat.substr(0, pat.size() - 1);
        return val.size() >= prefix.size() &&
               val.compare(0, prefix.size(), prefix) == 0;
    }
    if (first == 0) {
        std::string suffix = pat.substr(1);
        return val.size() >= suffix.size() &&
               val.compare(val.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    return val.find(pat.substr(0, first)) != std::string::npos;
}

std::vector<WorklistItem> WorklistIndex::Find(
    const std::unordered_map<std::string, std::string>& keys) const {

    auto get = [&](const char* k) -> std::string {
        auto it = keys.find(k);
        return it == keys.end() ? std::string{} : it->second;
    };

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<WorklistItem> out;
    for (const auto& w : items_) {
        if (!Matches(w.patientId, get("PatientID"))) continue;
        if (!Matches(w.patientName, get("PatientName"))) continue;
        if (!Matches(w.accessionNumber, get("AccessionNumber"))) continue;
        if (!Matches(w.modality, get("Modality"))) continue;
        if (!Matches(w.scheduledStationAet, get("ScheduledStationAETitle"))) continue;
        if (!Matches(w.scheduledProcedureStepStartDate, get("ScheduledProcedureStepStartDate"))) continue;
        out.push_back(w);
    }
    return out;
}

}  // namespace dicomserver
