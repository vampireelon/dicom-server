#include "dicomserver/AccessControl.hpp"
#include "dicomserver/Logging.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>

namespace dicomserver {

namespace {

std::string Lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string OpName(DicomOp op) {
    switch (op) {
        case DicomOp::Echo:    return "echo";
        case DicomOp::Store:   return "store";
        case DicomOp::Find:    return "find";
        case DicomOp::Move:    return "move";
        case DicomOp::Get:     return "get";
        case DicomOp::NAction: return "naction";
        case DicomOp::Mpps:    return "mpps";
    }
    return "";
}

}  // namespace

const char* ToString(DicomOp op) {
    static thread_local std::string s;
    s = OpName(op);
    return s.c_str();
}

bool ParseOp(std::string_view s, DicomOp& out) {
    auto l = Lower(std::string(s));
    if (l == "echo") { out = DicomOp::Echo; return true; }
    if (l == "store") { out = DicomOp::Store; return true; }
    if (l == "find" || l == "mwl") { out = DicomOp::Find; return true; }
    if (l == "move") { out = DicomOp::Move; return true; }
    if (l == "get") { out = DicomOp::Get; return true; }
    if (l == "naction" || l == "commitment") { out = DicomOp::NAction; return true; }
    if (l == "mpps") { out = DicomOp::Mpps; return true; }
    return false;
}

AccessControl& AccessControl::Instance() {
    static AccessControl ac;
    return ac;
}

void AccessControl::Configure(const ServerConfig& cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = cfg.aclEnabled;
    defaultPolicy_ = Lower(cfg.aclDefaultPolicy);
    if (defaultPolicy_ != "deny") defaultPolicy_ = "allow";
    rateLimitPerMinute_ = cfg.associationRateLimitPerMinute;
    rules_.clear();

    for (const auto& r : cfg.aclRules) {
        Rule rule;
        rule.callingAet = r.callingAet;
        for (const auto& op : r.allow) {
            rule.allow.insert(Lower(op));
        }
        rules_.push_back(std::move(rule));
    }

    DS_INFO("AccessControl  enabled={}  default={}  rules={}  rateLimit={}/min",
            enabled_, defaultPolicy_, rules_.size(), rateLimitPerMinute_);
}

bool AccessControl::Enabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

std::string AccessControl::Describe() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << "enabled=" << (enabled_ ? "true" : "false")
        << " default=" << defaultPolicy_
        << " rules=" << rules_.size()
        << " rateLimit=" << rateLimitPerMinute_;
    return oss.str();
}

bool AccessControl::AllowAssociation(std::string_view callingAet) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!enabled_) return true;

    const std::string aet(callingAet);
    bool matched = false;
    for (const auto& rule : rules_) {
        if (rule.callingAet == "*" || rule.callingAet == aet) {
            matched = true;
            if (!rule.allow.empty()) return true;
        }
    }
    if (matched) return true;
    return defaultPolicy_ == "allow";
}

bool AccessControl::AllowOperation(std::string_view callingAet, DicomOp op) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!enabled_) return true;

    const std::string aet(callingAet);
    const std::string opName = OpName(op);

    auto check = [&](const Rule& rule) -> int {
        if (rule.callingAet != "*" && rule.callingAet != aet) return -1;
        if (rule.allow.count("*") || rule.allow.count(opName)) return 1;
        return 0;
    };

    int exact = -1, star = -1;
    for (const auto& rule : rules_) {
        int r = check(rule);
        if (rule.callingAet == aet && r >= 0) exact = r;
        if (rule.callingAet == "*" && r >= 0) star = r;
    }
    if (exact >= 0) return exact == 1;
    if (star >= 0) return star == 1;
    return defaultPolicy_ == "allow";
}

bool AccessControl::CheckRateLimit(std::string_view callingAet) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (rateLimitPerMinute_ == 0) return true;

    using clock = std::chrono::steady_clock;
    const auto now = clock::now();
    const auto window = std::chrono::minutes(1);
    auto& q = rateMap_[std::string(callingAet)];
    q.erase(std::remove_if(q.begin(), q.end(),
                           [&](const auto& t) { return now - t > window; }),
            q.end());
    if (q.size() >= rateLimitPerMinute_) {
        DS_WARN("Rate limit exceeded for AE '{}' ({}/min)", callingAet, rateLimitPerMinute_);
        return false;
    }
    q.push_back(now);
    return true;
}

AnonymizedView AnonymizeIdentity(std::string_view patientId,
                                 std::string_view patientName,
                                 std::string_view accessionNumber,
                                 std::string_view /*birthDate*/) {
    auto hash = [](std::string_view s) -> std::string {
        std::size_t h = std::hash<std::string_view>{}(s);
        std::ostringstream oss;
        oss << std::hex << h;
        return oss.str().substr(0, 12);
    };
    AnonymizedView v;
    v.patientId = patientId.empty() ? "ANON" : ("ID-" + hash(patientId));
    v.patientName = "ANONYMIZED^PATIENT";
    v.accessionNumber = accessionNumber.empty() ? "" : ("ACC-" + hash(accessionNumber));
    v.patientBirthDate = "";
    return v;
}

}  // namespace dicomserver
