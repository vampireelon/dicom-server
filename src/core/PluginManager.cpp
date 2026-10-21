#include "dicomserver/PluginManager.hpp"
#include "dicomserver/Logging.hpp"

#include <cstring>
#include <filesystem>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace fs = std::filesystem;

namespace dicomserver {

namespace {

void CopyField(char* dst, size_t cap, const std::string& src) {
    if (cap == 0) return;
    std::strncpy(dst, src.c_str(), cap - 1);
    dst[cap - 1] = '\0';
}

void* SysLoad(const char* path) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(LoadLibraryA(path));
#else
    return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
}

void SysUnload(void* h) {
    if (!h) return;
#if defined(_WIN32)
    FreeLibrary(reinterpret_cast<HMODULE>(h));
#else
    dlclose(h);
#endif
}

void* SysSym(void* h, const char* name) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(h), name));
#else
    return dlsym(h, name);
#endif
}

const char* SysError() {
#if defined(_WIN32)
    return "LoadLibrary/GetProcAddress failed";
#else
    const char* e = dlerror();
    return e ? e : "unknown dlerror";
#endif
}

}  // namespace

PluginManager& PluginManager::Instance() {
    static PluginManager m;
    return m;
}

PluginManager::~PluginManager() {
    UnloadAll();
}

void PluginManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& p : plugins_) {
        if (p.fini) {
            try { p.fini(); } catch (...) {}
        }
        SysUnload(p.handle);
        DS_INFO("Plugin unloaded: {} ({})", p.name, p.path);
    }
    plugins_.clear();
}

std::size_t PluginManager::PluginCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return plugins_.size();
}

std::vector<std::string> PluginManager::PluginNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const auto& p : plugins_) names.push_back(p.name);
    return names;
}

void PluginManager::LoadDirectory(std::string_view dir) {
    if (dir.empty()) return;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        DS_INFO("Plugin directory '{}' not found – skipping", dir);
        return;
    }
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
#if defined(_WIN32)
        if (ext != ".dll") continue;
#else
        if (ext != ".so") continue;
#endif
        LoadOne(entry.path().string());
    }
}

void PluginManager::LoadOne(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& p : plugins_) {
        if (p.path == path) return;
    }

    void* handle = SysLoad(path.c_str());
    if (!handle) {
        DS_ERROR("Failed to load plugin '{}': {}", path, SysError());
        return;
    }

    Plugin p;
    p.handle = handle;
    p.path = path;
    p.nameFn = reinterpret_cast<Plugin::NameFn>(SysSym(handle, "ds_plugin_name"));
    p.init = reinterpret_cast<Plugin::InitFn>(SysSym(handle, "ds_plugin_init"));
    p.fini = reinterpret_cast<Plugin::FiniFn>(SysSym(handle, "ds_plugin_fini"));
    p.onStored = reinterpret_cast<Plugin::OnStoredFn>(SysSym(handle, "ds_on_stored_instance"));
    p.onFind = reinterpret_cast<Plugin::OnFindFn>(SysSym(handle, "ds_on_find_filter"));
    p.onMove = reinterpret_cast<Plugin::OnMoveFn>(SysSym(handle, "ds_on_move_destination"));

    if (!p.nameFn || !p.init || !p.fini) {
        DS_ERROR("Plugin '{}' missing required exports (ds_plugin_name/init/fini)", path);
        SysUnload(handle);
        return;
    }

    p.name = p.nameFn() ? p.nameFn() : path;
    int rc = 0;
    try {
        rc = p.init();
    } catch (const std::exception& e) {
        DS_ERROR("Plugin '{}' init threw: {}", p.name, e.what());
        SysUnload(handle);
        return;
    }
    if (rc != 0) {
        DS_ERROR("Plugin '{}' init returned {}", p.name, rc);
        SysUnload(handle);
        return;
    }

    DS_INFO("Plugin loaded: {}  hooks={{stored:{}, find:{}, move:{}}}  path={}",
            p.name,
            p.onStored ? "yes" : "no",
            p.onFind ? "yes" : "no",
            p.onMove ? "yes" : "no",
            path);
    plugins_.push_back(std::move(p));
}

void PluginManager::OnStoredInstance(const IndexedInstance& inst,
                                     std::string_view callingAet,
                                     std::string_view filePath) {
    ds_stored_instance_t info{};
    info.sop_class_uid = inst.sopClassUid.c_str();
    info.sop_instance_uid = inst.sopInstanceUid.c_str();
    info.study_instance_uid = inst.studyInstanceUid.c_str();
    info.series_instance_uid = inst.seriesInstanceUid.c_str();
    info.patient_id = inst.patientId.c_str();
    info.patient_name = inst.patientName.c_str();
    info.modality = inst.modality.c_str();
    std::string fp(filePath);
    std::string aet(callingAet);
    info.file_path = fp.c_str();
    info.calling_aet = aet.c_str();

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& p : plugins_) {
        if (!p.onStored) continue;
        try {
            p.onStored(&info);
        } catch (const std::exception& e) {
            DS_WARN("Plugin '{}' OnStored threw: {}", p.name, e.what());
        }
    }
}

std::vector<IndexedInstance> PluginManager::FilterFindResults(
    std::string_view level,
    std::vector<IndexedInstance> matches) {
    std::string levelStr(level);
    std::lock_guard<std::mutex> lock(mutex_);
    if (plugins_.empty()) return matches;

    bool anyFilter = false;
    for (const auto& p : plugins_) {
        if (p.onFind) { anyFilter = true; break; }
    }
    if (!anyFilter) return matches;

    std::vector<IndexedInstance> out;
    out.reserve(matches.size());
    for (auto& inst : matches) {
        ds_find_row_t row{};
        row.keep = 1;
        CopyField(row.patient_id, DS_FIND_FIELD_CAP, inst.patientId);
        CopyField(row.patient_name, DS_FIND_FIELD_CAP, inst.patientName);
        CopyField(row.study_instance_uid, DS_FIND_FIELD_CAP, inst.studyInstanceUid);
        CopyField(row.series_instance_uid, DS_FIND_FIELD_CAP, inst.seriesInstanceUid);
        CopyField(row.sop_instance_uid, DS_FIND_FIELD_CAP, inst.sopInstanceUid);
        CopyField(row.modality, DS_FIND_FIELD_CAP, inst.modality);
        CopyField(row.accession_number, DS_FIND_FIELD_CAP, inst.accessionNumber);
        CopyField(row.study_date, DS_FIND_FIELD_CAP, inst.studyDate);

        for (auto& p : plugins_) {
            if (!p.onFind) continue;
            try {
                p.onFind(levelStr.c_str(), &row);
            } catch (const std::exception& e) {
                DS_WARN("Plugin '{}' OnFind threw: {}", p.name, e.what());
            }
        }
        if (!row.keep) continue;

        inst.patientId = row.patient_id;
        inst.patientName = row.patient_name;
        inst.studyInstanceUid = row.study_instance_uid;
        inst.seriesInstanceUid = row.series_instance_uid;
        inst.sopInstanceUid = row.sop_instance_uid;
        inst.modality = row.modality;
        inst.accessionNumber = row.accession_number;
        inst.studyDate = row.study_date;
        out.push_back(std::move(inst));
    }
    return out;
}

bool PluginManager::ResolveMoveDestination(std::string_view aet,
                                           std::string& hostOut,
                                           uint16_t& portOut) {
    std::string aetStr(aet);
    std::lock_guard<std::mutex> lock(mutex_);
    char hostBuf[256];
    for (auto& p : plugins_) {
        if (!p.onMove) continue;
        hostBuf[0] = '\0';
        int port = 0;
        int rc = 0;
        try {
            rc = p.onMove(aetStr.c_str(), hostBuf, sizeof(hostBuf), &port);
        } catch (const std::exception& e) {
            DS_WARN("Plugin '{}' OnMove threw: {}", p.name, e.what());
            continue;
        }
        if (rc == 1 && hostBuf[0] != '\0' && port > 0 && port <= 65535) {
            hostOut = hostBuf;
            portOut = static_cast<uint16_t>(port);
            DS_INFO("Plugin '{}' resolved MOVE dest {} -> {}:{}",
                    p.name, aetStr, hostOut, portOut);
            return true;
        }
    }
    return false;
}

}  // namespace dicomserver
