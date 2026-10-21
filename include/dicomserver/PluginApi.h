/**
 * C ABI for dicom-server plugins (Phase 9).
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  define DS_PLUGIN_EXPORT __declspec(dllexport)
#else
#  define DS_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

typedef struct ds_stored_instance {
    const char* sop_class_uid;
    const char* sop_instance_uid;
    const char* study_instance_uid;
    const char* series_instance_uid;
    const char* patient_id;
    const char* patient_name;
    const char* modality;
    const char* file_path;
    const char* calling_aet;
} ds_stored_instance_t;

#define DS_FIND_FIELD_CAP 256

typedef struct ds_find_row {
    char patient_id[DS_FIND_FIELD_CAP];
    char patient_name[DS_FIND_FIELD_CAP];
    char study_instance_uid[DS_FIND_FIELD_CAP];
    char series_instance_uid[DS_FIND_FIELD_CAP];
    char sop_instance_uid[DS_FIND_FIELD_CAP];
    char modality[DS_FIND_FIELD_CAP];
    char accession_number[DS_FIND_FIELD_CAP];
    char study_date[DS_FIND_FIELD_CAP];
    int  keep;
} ds_find_row_t;

DS_PLUGIN_EXPORT const char* ds_plugin_name(void);
DS_PLUGIN_EXPORT int ds_plugin_init(void);
DS_PLUGIN_EXPORT void ds_plugin_fini(void);
DS_PLUGIN_EXPORT void ds_on_stored_instance(const ds_stored_instance_t* info);
DS_PLUGIN_EXPORT void ds_on_find_filter(const char* level, ds_find_row_t* row);
DS_PLUGIN_EXPORT int ds_on_move_destination(
    const char* aet, char* host_out, size_t host_out_len, int* port_out);

#ifdef __cplusplus
}
#endif
