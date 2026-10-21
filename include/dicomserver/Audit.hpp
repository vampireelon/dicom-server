#pragma once

#include <fstream>
#include <mutex>
#include <string>
#include <string_view>

namespace dicomserver {

class Audit {
public:
    static Audit& Instance();
    void Open(std::string_view path, std::size_t maxSizeBytes = 100 * 1024 * 1024,
              bool hashChain = false);
    void Close();
    void Event(std::string_view event, std::string_view details = {});
    void AssociationAccepted(std::string_view calling, std::string_view called);
    void AssociationRejected(std::string_view calling, std::string_view reason);
    void Store(std::string_view calling, std::string_view sopInstance,
               std::string_view path, bool success);
    void Find(std::string_view calling, std::string_view level, std::size_t matches);
    void Move(std::string_view calling, std::string_view dest,
              std::size_t completed, std::size_t failed);
    void StorageCommitment(std::string_view calling,
                           std::size_t success, std::size_t failed);

private:
    Audit() = default;
    void WriteLine(std::string_view line);
    void RotateIfNeeded();
    std::mutex   mutex_;
    std::ofstream file_;
    std::string  path_;
    std::size_t  maxSizeBytes_{100 * 1024 * 1024};
    bool         hashChain_{false};
    std::string  lastHash_{"GENESIS"};
};

}  // namespace dicomserver
