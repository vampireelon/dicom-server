#pragma once

#include "Config.hpp"
#include "InstanceIndex.hpp"
#include "StoreHandler.hpp"
#include "WorklistIndex.hpp"
#include "MppsIndex.hpp"
#include "AccessControl.hpp"
#include "Audit.hpp"
#include "PluginManager.hpp"
#include "Metrics.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Forward DCMTK types to keep header light
struct T_ASC_Association;
struct T_DIMSE_Message;
typedef unsigned char T_ASC_PresentationContextID;
typedef unsigned short OFCondition;  // simplified; real code uses DCMTK OFCondition

namespace dicomserver {

class DicomServer {
public:
    explicit DicomServer();
    ~DicomServer();

    DicomServer(const DicomServer&) = delete;
    DicomServer& operator=(const DicomServer&) = delete;

    void Start();
    void Stop();
    bool IsRunning() const;

    void Reload();

    InstanceIndex& Index();
    WorklistIndex& Worklist();
    MppsIndex& Mpps();
    AccessControl& Acl();
    AuditLog& Audit();
    PluginManager& Plugins();
    Metrics& GetMetrics();

private:
    void AcceptLoop();
    void OnAssociation(int clientFd, const std::string& peerIp);

    std::atomic<bool> running_{false};
    int listenFd_{-1};
    std::thread acceptThread_;
    std::mutex mu_;

    std::unique_ptr<InstanceIndex> index_;
    std::unique_ptr<WorklistIndex> worklist_;
    std::unique_ptr<MppsIndex> mpps_;
    std::unique_ptr<AccessControl> acl_;
    std::unique_ptr<AuditLog> audit_;
    std::unique_ptr<PluginManager> plugins_;
    std::unique_ptr<StoreHandler> storeHandler_;
    Metrics metrics_;
};

}  // namespace dicomserver
