# Upload status

Repository: https://github.com/vampireelon/dicom-server

## Already on `main`

- Build: `CMakeLists.txt`, `Dockerfile`, `docker-compose.yml`
- Config: `config/server.json`, `config/server.docker.json`
- License / README / .gitignore
- Most public headers under `include/dicomserver/`
- `src/core/Metrics.cpp`, unit tests skeleton

## Still to push (implementation)

Core `.cpp` under `src/core/`, `src/dicom/`, `src/http/`, remaining headers
(`DicomServer.hpp`, `InstanceIndex.hpp`, …), and `plugins/example_logger.cpp`.

These files exist in the development workspace and can be committed in a follow-up push.

After the full tree is present:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/dicom-server --config config/server.json
```
