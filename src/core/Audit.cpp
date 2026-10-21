#include "dicomserver/Audit.hpp"
#include "dicomserver/Logging.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <functional>
#include <sstream>

namespace dicomserver {

namespace {

std::string NowIso() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    std::time_t t = clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

}  // namespace

Audit& Audit::Instance() {
    static Audit a;
    return a;
}

void Audit::Open(std::string_view path, std::size_t maxSizeBytes, bool hashChain) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) file_.close();
    path_ = std::string(path);
    maxSizeBytes_ = maxSizeBytes ? maxSizeBytes : 100 * 1024 * 1024;
    hashChain_ = hashChain;
    lastHash_ = "GENESIS";
    if (path_.empty()) return;
    file_.open(path_, std::ios::app);
    if (!file_) {
        DS_ERROR("Cannot open audit log: {}", path_);
        path_.clear();
    } else {
        DS_INFO("Audit log: {}  maxSize={}MB", path_, maxSizeBytes_ / (1024 * 1024));
    }
}

void Audit::RotateIfNeeded() {
    if (path_.empty() || !file_.is_open()) return;
    file_.flush();
    std::error_code ec;
    auto sz = std::filesystem::file_size(path_, ec);
    if (ec || sz < maxSizeBytes_) return;
    file_.close();
    auto rotated = path_ + ".1";
    std::filesystem::remove(rotated, ec);
    std::filesystem::rename(path_, rotated, ec);
    file_.open(path_, std::ios::app);
    DS_INFO("Audit log rotated -> {}", rotated);
}

void Audit::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) file_.close();
}

void Audit::WriteLine(std::string_view line) {
    std::string out(line);
    if (hashChain_) {
        std::size_t h = std::hash<std::string>{}(lastHash_ + "|" + out);
        std::ostringstream hs;
        hs << std::hex << h;
        std::string hash = hs.str();
        out = out + " | prev=" + lastHash_ + " | hash=" + hash;
        lastHash_ = hash;
    }
    DS_INFO("[AUDIT] {}", out);
    if (file_.is_open()) {
        file_ << out << '\n';
        file_.flush();
        RotateIfNeeded();
    }
}

void Audit::Event(std::string_view event, std::string_view details) {
    std::ostringstream oss;
    oss << NowIso() << " | " << event;
    if (!details.empty()) oss << " | " << details;
    std::lock_guard<std::mutex> lock(mutex_);
    WriteLine(oss.str());
}

void Audit::AssociationAccepted(std::string_view calling, std::string_view called) {
    Event("ASSOC_ACCEPT", std::string("calling=") + std::string(calling) +
                              " called=" + std::string(called));
}

void Audit::AssociationRejected(std::string_view calling, std::string_view reason) {
    Event("ASSOC_REJECT", std::string("calling=") + std::string(calling) +
                              " reason=" + std::string(reason));
}

void Audit::Store(std::string_view calling, std::string_view sopInstance,
                  std::string_view path, bool success) {
    Event(success ? "C_STORE_OK" : "C_STORE_FAIL",
          std::string("calling=") + std::string(calling) +
              " sop=" + std::string(sopInstance) +
              " path=" + std::string(path));
}

void Audit::Find(std::string_view calling, std::string_view level, std::size_t matches) {
    Event("C_FIND", std::string("calling=") + std::string(calling) +
                        " level=" + std::string(level) +
                        " matches=" + std::to_string(matches));
}

void Audit::Move(std::string_view calling, std::string_view dest,
                 std::size_t completed, std::size_t failed) {
    Event("C_MOVE", std::string("calling=") + std::string(calling) +
                        " dest=" + std::string(dest) +
                        " completed=" + std::to_string(completed) +
                        " failed=" + std::to_string(failed));
}

void Audit::StorageCommitment(std::string_view calling,
                              std::size_t success, std::size_t failed) {
    Event("N_ACTION_SC", std::string("calling=") + std::string(calling) +
                             " success=" + std::to_string(success) +
                             " failed=" + std::to_string(failed));
}

}  // namespace dicomserver
