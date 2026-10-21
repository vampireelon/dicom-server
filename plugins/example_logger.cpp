/**
 * Example dicom-server plugin: logs store events and tags FIND rows.
 */
#include "dicomserver/PluginApi.h"

#include <cstdio>
#include <cstring>

static FILE* g_log = nullptr;

extern "C" {

DS_PLUGIN_EXPORT const char* ds_plugin_name(void) {
    return "example_logger";
}

DS_PLUGIN_EXPORT int ds_plugin_init(void) {
    g_log = std::fopen("/tmp/dicom_plugin_example.log", "a");
    if (g_log) {
        std::fprintf(g_log, "[init] example_logger loaded\n");
        std::fflush(g_log);
    }
    return 0;
}

DS_PLUGIN_EXPORT void ds_plugin_fini(void) {
    if (g_log) {
        std::fprintf(g_log, "[fini] example_logger unloaded\n");
        std::fclose(g_log);
        g_log = nullptr;
    }
}

DS_PLUGIN_EXPORT void ds_on_stored_instance(const ds_stored_instance_t* info) {
    if (!info || !g_log) return;
    std::fprintf(g_log,
                 "[stored] calling=%s patient=%s sop=%s path=%s\n",
                 info->calling_aet ? info->calling_aet : "",
                 info->patient_id ? info->patient_id : "",
                 info->sop_instance_uid ? info->sop_instance_uid : "",
                 info->file_path ? info->file_path : "");
    std::fflush(g_log);
}

DS_PLUGIN_EXPORT void ds_on_find_filter(const char* level, ds_find_row_t* row) {
    if (!row) return;
    if (level && std::strcmp(level, "STUDY") == 0) {
        (void)row;
    }
    if (g_log) {
        std::fprintf(g_log, "[find] level=%s patient=%s study=%s keep=%d\n",
                     level ? level : "",
                     row->patient_id,
                     row->study_instance_uid,
                     row->keep);
        std::fflush(g_log);
    }
}

DS_PLUGIN_EXPORT int ds_on_move_destination(
    const char* /*aet*/, char* /*host_out*/, size_t /*host_out_len*/, int* /*port_out*/) {
    return 0;
}

}  // extern "C"
