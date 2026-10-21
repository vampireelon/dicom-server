#pragma once

#include "dicomserver/Config.hpp"

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dicomserver {

enum class DicomOp {
    Echo,
    Store,
    Find,
    Move,
    Get,
    NAction,
    Mpps,
};

const char* ToString(DicomOp op);
bool ParseOp(std::string_view s, DicomOp& out);

class AccessControl {
public:
    static AccessControl& Instance();
    void Configure(const ServerConfig& cfg);
    [[nodiscard]] bool AllowAssociation(std::string_view callingAet) const;
    [[nodiscard]] bool AllowOperation(std::string_view callingAet, DicomOp op) const;
    bool CheckRateLimit(std::string_view callingAet);
    [[nodiscard]] bool Enabled() const;
    [[nodiscard]] std::string Describe() const;

private:
    AccessControl() = default;
    struct Rule {
        std::string callingAet;
        std::unordered_set<std::string> allow;
    };
    mutable std::mutex mutex_;
    bool enabled_{false};
    std::string defaultPolicy_{"allow"};
    std::vector<Rule> rules_;
    unsigned rateLimitPerMinute_{0};
    std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> rateMap_;
};

struct AnonymizedView {
    std::string patientId;
    std::string patientName;
    std::string accessionNumber;
    std::string patientBirthDate;
};

AnonymizedView AnonymizeIdentity(std::string_view patientId,
                                 std::string_view patientName,
                                 std::string_view accessionNumber,
                                 std::string_view birthDate = {});

}  // namespace dicomserver
