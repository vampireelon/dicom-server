#!/usr/bin/env bash
# Restore large sources from multi-part gzipped base64 under .source_chunks/
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
python3 - << 'PY'
import base64, gzip, re
from pathlib import Path
from collections import defaultdict

chunks = Path(".source_chunks")
groups = defaultdict(list)
for p in sorted(chunks.glob("*.part*.b64")):
    m = re.match(r"(.+)\.part(\d+)\.b64$", p.name)
    if not m:
        continue
    prefix, idx = m.group(1), int(m.group(2))
    groups[prefix].append((idx, p.read_text().strip()))

dest_map = {
    "WebUi.cpp.gz": "src/http/WebUi.cpp",
    "HttpServer.cpp.gz": "src/http/HttpServer.cpp",
    "DicomWeb.cpp.gz": "src/http/DicomWeb.cpp",
    "InstanceIndex.cpp.gz": "src/dicom/InstanceIndex.cpp",
    "DicomServer.cpp.gz": "src/dicom/DicomServer.cpp",
}

for prefix, parts in sorted(groups.items()):
    dest = dest_map.get(prefix)
    if not dest:
        print(f"unknown prefix {prefix}, skip")
        continue
    parts.sort(key=lambda x: x[0])
    b64 = "".join(t for _, t in parts)
    data = gzip.decompress(base64.b64decode(b64))
    out = Path(dest)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(data)
    print(f"Wrote {dest} ({len(data)} bytes from {len(parts)} parts)")
print("Done.")
PY
