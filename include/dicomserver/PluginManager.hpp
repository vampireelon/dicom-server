#pragma once

#include "dicomserver/PluginApi.h"
#include "dicomserver/InstanceIndex.hpp"
#include "dicomserver/Config.hpp"

#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace dicomserver {

class PluginManager {
public:
    static PluginManager& Instance();
    void LoadDirectory(std::string_view dir);
    void UnloadAll();
    [[nodiscard]] std::size_t PluginCount() const;
    [[nodiscard]] std::vector<std::string> PluginNames() const;
    void OnStoredInstance(const IndexedInstance& inst, std::string_view callingAet,
                          std::string_view filePath);
    std::vector<IndexedInstance> FilterFindResults(
        std::string_view level,
        std::vector<IndexedInstance> matches);
    bool ResolveMoveDestination(std::string_view aet,
                                std::string& hostOut,
                                uint16_t& portOut);

private:
    PluginManager() = default;
    ~PluginManager();
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    struct Plugin {
        void* handle = nullptr;
        std::string path;
        std::string name;
        using InitFn = int (*)();
        using FiniFn = void (*)();
        using NameFn = const char* (*)();
        using OnStoredFn = void (*)(const ds_stored_instance_t*);
        using OnFindFn = void (*)(const char*, ds_find_row_t*);
        using OnMoveFn = int (*)(const char*, char*, size_t, int*);
        InitFn init = nullptr;
        FiniFn fini = nullptr;
        NameFn nameFn = nullptr;
        OnStoredFn onStored = nullptr;
        OnFindFn onFind = nullptr;
        OnMoveFn onMove = nullptr;
    };

    void LoadOne(const std::string& path);
    mutable std::mutex mutex_;
    std::vector<Plugin> plugins_;
};

}  // namespace dicomserver
