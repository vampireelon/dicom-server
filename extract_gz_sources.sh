#!/usr/bin/env bash
# Restore large sources from gzipped base64 under .source_chunks/
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
python3 - << 'PY'
import base64, gzip
from pathlib import Path
mapping = {
    "WebUi.cpp.gz.b64": "src/http/WebUi.cpp",
    "HttpServer.cpp.gz.b64": "src/http/HttpServer.cpp",
    "DicomWeb.cpp.gz.b64": "src/http/DicomWeb.cpp",
    "InstanceIndex.cpp.gz.b64": "src/dicom/InstanceIndex.cpp",
    "DicomServer.cpp.gz.b64": "src/dicom/DicomServer.cpp",
}
chunks = Path(".source_chunks")
for name, dest in mapping.items():
    p = chunks / name
    if not p.exists():
        print(f"skip missing {name}")
        continue
    data = gzip.decompress(base64.b64decode(p.read_text().strip()))
    out = Path(dest)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(data)
    print(f"Wrote {dest} ({len(data)} bytes)")
print("Done.")
PY
