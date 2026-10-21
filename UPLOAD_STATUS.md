# Upload Status

**Repo:** https://github.com/vampireelon/dicom-server

## How to restore large sources

```bash
git clone https://github.com/vampireelon/dicom-server.git
cd dicom-server
bash extract_gz_sources.sh
```

This reconstructs:
- `src/http/WebUi.cpp`
- `src/http/HttpServer.cpp`
- `src/http/DicomWeb.cpp`
- (and InstanceIndex / DicomServer when all parts are present)

## Sources on main as normal files
- All headers, config, Docker, CMake
- `src/main.cpp`
- `src/core/*` including PluginManager
- `src/dicom/StoreHandler, WorklistIndex, MppsIndex, DicomScu`
- plugins, tests

## Large sources via `.source_chunks/*.gz.part*.b64`
| File | Status |
|------|--------|
| WebUi.cpp | 3/3 parts uploaded |
| HttpServer.cpp | 3/3 parts uploaded |
| DicomWeb.cpp | 3/3 parts uploaded |
| InstanceIndex.cpp | in progress |
| DicomServer.cpp | pending |

After `bash extract_gz_sources.sh` the missing `.cpp` files appear under `src/`.
