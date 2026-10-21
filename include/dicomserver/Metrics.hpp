#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace dicomserver {

class Metrics {
public:
    static Metrics& Instance();

    std::atomic<uint64_t> associationsAccepted{0};
    std::atomic<uint64_t> associationsRejected{0};
    std::atomic<uint64_t> associationsActive{0};

    std::atomic<uint64_t> cEcho{0};
    std::atomic<uint64_t> cStoreSuccess{0};
    std::atomic<uint64_t> cStoreFailed{0};
    std::atomic<uint64_t> cFind{0};
    std::atomic<uint64_t> cMoveSuccess{0};
    std::atomic<uint64_t> cMoveFailed{0};
    std::atomic<uint64_t> cGetSuccess{0};
    std::atomic<uint64_t> cGetFailed{0};
    std::atomic<uint64_t> nAction{0};
    std::atomic<uint64_t> mwlFind{0};

    std::atomic<uint64_t> dimseErrors{0};
    std::atomic<uint64_t> storageErrors{0};

    std::string Snapshot() const;
    void Reset();

private:
    Metrics() = default;
};

}  // namespace dicomserver
