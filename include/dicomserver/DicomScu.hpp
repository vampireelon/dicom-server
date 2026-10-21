#pragma once

#include "dicomserver/Config.hpp"
#include "dicomserver/InstanceIndex.hpp"

#include <string>
#include <vector>

namespace dicomserver {

struct StoreScuResult {
    unsigned completed = 0;
    unsigned failed    = 0;
    unsigned warning   = 0;
};

StoreScuResult StoreInstancesToRemote(
    const std::string& ourAet,
    const ServerConfig::RemoteModality& dest,
    const std::vector<IndexedInstance>& instances,
    uint32_t timeoutSec);

StoreScuResult StoreInstancesOnAssociation(
    void* assoc,
    const std::vector<IndexedInstance>& instances,
    uint32_t timeoutSec);

const ServerConfig::RemoteModality* FindModality(
    const std::vector<ServerConfig::RemoteModality>& modalities,
    const std::string& aet);

}  // namespace dicomserver
