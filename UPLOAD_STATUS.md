# Upload Status

**Repo:** https://github.com/vampireelon/dicom-server

## Restore large sources

```bash
git clone https://github.com/vampireelon/dicom-server.git
cd dicom-server
bash extract_gz_sources.sh
```

This reconstructs under `src/`:

| File | Parts |
|------|-------|
| `src/http/WebUi.cpp` | 3 |
| `src/http/HttpServer.cpp` | 3 |
| `src/http/DicomWeb.cpp` | 3 |
| `src/dicom/InstanceIndex.cpp` | 4 |
| `src/dicom/DicomServer.cpp` | 8 |

## Normal source files (direct on main)
- Headers, CMake, Docker, config
- `src/main.cpp`
- `src/core/*` (including PluginManager)
- `src/dicom/StoreHandler, WorklistIndex, MppsIndex, DicomScu`
- plugins, tests

## Notes
- Large files are stored as multi-part gzip+base64 under `.source_chunks/`.
- Run `extract_gz_sources.sh` after clone before building.
- If extract fails on a file, re-download that file's parts or copy from the development workspace.
