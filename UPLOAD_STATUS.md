# Upload Status

**Repo:** https://github.com/vampireelon/dicom-server

## Done (exact or near-exact source on `main`)

### Build / config
- LICENSE, README.md, .gitignore, .dockerignore
- CMakeLists.txt, Dockerfile, docker-compose.yml
- config/server.json, config/server.docker.json

### Headers (`include/dicomserver/`)
- All public headers including DicomServer.hpp, InstanceIndex.hpp, PluginApi.h, etc.

### Sources uploaded
- `src/main.cpp`
- `src/core/`: AccessControl, Audit, Config, Exception, Logging, Metrics, **PluginManager**
- `src/dicom/`: StoreHandler, WorklistIndex, MppsIndex, **DicomScu**
- `plugins/example_logger.cpp`
- `tests/*`

## Still missing (large files)

| Path | ~Size |
|------|-------|
| `src/dicom/DicomServer.cpp` | 60 KB |
| `src/dicom/InstanceIndex.cpp` | 24 KB |
| `src/http/HttpServer.cpp` | 16 KB |
| `src/http/DicomWeb.cpp` | 20 KB |
| `src/http/WebUi.cpp` | 14 KB |

These remain only in the development workspace. GitHub API push of multi-10KB single files is constrained; continue batch upload or copy from the local tree.

## Local full tree

```
/home/workdir/artifacts/dicom-server/
```

After cloning, you can merge remaining files from a local copy of that tree.
