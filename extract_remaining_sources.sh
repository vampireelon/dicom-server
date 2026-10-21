#!/usr/bin/env bash
# Reconstruct remaining source files from .source_chunks/*.b64
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
export ROOT
CHUNKS="$ROOT/.source_chunks"
if [[ ! -d "$CHUNKS" ]]; then
  echo "No .source_chunks directory"
  exit 1
fi
python3 - << 'PY'
import base64, os
from pathlib import Path
from collections import defaultdict
root = Path(os.environ.get("ROOT", "."))
chunks = root / ".source_chunks"
groups = defaultdict(list)
for p in sorted(chunks.glob("*.b64")):
    name = p.name
    if ".part" not in name:
        continue
    base, part = name.rsplit(".part", 1)
    part_num = int(part.split(".")[0])
    path = base.replace("__", "/")
    groups[path].append((part_num, p.read_text()))
for path, parts in groups.items():
    parts.sort(key=lambda x: x[0])
    b64 = "".join(p[1] for p in parts)
    data = base64.b64decode(b64)
    out = root / path
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(data)
    print(f"Wrote {path} ({len(data)} bytes)")
print("Done.")
PY
