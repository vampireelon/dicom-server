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

namespace dicomserver {

class DicomServer {
public:
    explicit DicomServer(Config config);
    ~DicomServer();

    DicomServer(const DicomServer&) = delete;
    DicomServer& operator=(const DicomServer&) = delete;

    void Start();
    void Stop();
    bool IsRunning() const { return running_.load(); }

    // Hot-reload configuration (SIGHUP)
    void Reload(const Config& newConfig);

    // Accessors for HTTP layer / plugins
    InstanceIndex& GetIndex() { return *index_; }
    const InstanceIndex& GetIndex() const { return *index_; }
    WorklistIndex& GetWorklist() { return *worklist_; }
    MppsIndex& GetMpps() { return *mpps_; }
    AccessControl& GetAcl() { return *acl_; }
    AuditLog& GetAudit() { return *audit_; }
    PluginManager& GetPlugins() { return *plugins_; }
    Metrics& GetMetrics() { return metrics_; }
    const Config& GetConfig() const { return config_; }

private:
    void AcceptLoop();
    void HandleAssociation(int sock, const std::string& peerAe, const std::string& peerIp);
    void ProcessDimse(void* assoc /* T_ASC_Association* */, const std::string& callingAe);

    // DIMSE service handlers
    OFCondition HandleCEcho(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId);
    OFCondition HandleCStore(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                             const std::string& callingAe);
    OFCondition HandleCFind(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                            const std::string& callingAe);
    OFCondition HandleCMove(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                            const std::string& callingAe);
    OFCondition HandleCGet(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                           const std::string& callingAe);
    OFCondition HandleNCreate(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                              const std::string& callingAe);
    OFCondition HandleNSet(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                           const std::string& callingAe);
    OFCondition HandleNAction(T_ASC_Association* assoc, T_DIMSE_Message* msg, T_ASC_PresentationContextID pcId,
                              const std::string& callingAe);

    Config config_;
    std::unique_ptr<InstanceIndex> index_;
    std::unique_ptr<WorklistIndex> worklist_;
    std::unique_ptr<MppsIndex> mpps_;
    std::unique_ptr<AccessControl> acl_;
    std::unique_ptr<AuditLog> audit_;
    std::unique_ptr<PluginManager> plugins_;
    std::unique_ptr<StoreHandler> storeHandler_;
    Metrics metrics_;

    std::atomic<bool> running_{false};
    int listenFd_{-1};
    std::thread acceptThread_;
    std::mutex configMutex_;
};

}  // namespace dicomserver
