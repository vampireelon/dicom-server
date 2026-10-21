#include "dicomserver/StoreHandler.hpp"
#include "dicomserver/InstanceIndex.hpp"
#include "dicomserver/Logging.hpp"

#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace dicomserver {

FileStoreRequestHandler::FileStoreRequestHandler(std::string storageDir)
    : storageDir_(std::move(storageDir)) {}

StoreStatus FileStoreRequestHandler::HandleStore(StoreInstanceInfo& info) {
    const std::string study  = info.studyInstanceUid.empty()
                                   ? "_unknown_study"
                                   : info.studyInstanceUid;
    const std::string series = info.seriesInstanceUid.empty()
                                   ? "_unknown_series"
                                   : info.seriesInstanceUid;
    const std::string sop    = info.sopInstanceUid.empty()
                                   ? "_unknown_instance"
                                   : info.sopInstanceUid;

    fs::path destDir = fs::path(storageDir_) / study / series;
    fs::path destFile = destDir / (sop + ".dcm");

    std::error_code ec;
    fs::create_directories(destDir, ec);
    if (ec) {
        DS_ERROR("Cannot create storage directory {}: {}",
                 destDir.string(), ec.message());
        return StoreStatus::RefusedOutOfResources;
    }

    if (!info.filePath.empty() && fs::exists(info.filePath)) {
        fs::rename(info.filePath, destFile, ec);
        if (ec) {
            fs::copy_file(info.filePath, destFile,
                          fs::copy_options::overwrite_existing, ec);
            if (ec) {
                DS_ERROR("Cannot write {}: {}", destFile.string(), ec.message());
                return StoreStatus::RefusedOutOfResources;
            }
            fs::remove(info.filePath);
        }
    } else {
        DS_ERROR("Temporary receive file missing: {}", info.filePath);
        return StoreStatus::ErrorCannotUnderstand;
    }

    info.filePath = destFile.string();
    InstanceIndex::Instance().AddFromStore(info);

    DS_INFO("Stored instance  PatientID={}  Study={}  Series={}  SOP={}  → {}",
            info.patientId,
            study.substr(0, 24),
            series.substr(0, 24),
            sop.substr(0, 24),
            destFile.string());

    return StoreStatus::Success;
}

}  // namespace dicomserver
