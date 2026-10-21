#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
if ! ls .source_chunks/remaining_sources.part*.b64 >/dev/null 2>&1; then
  echo "No remaining_sources parts found"
  exit 1
fi
cat .source_chunks/remaining_sources.part*.b64 | base64 -d | tar xzf -
echo "Extracted:"
ls -la src/http/WebUi.cpp src/http/HttpServer.cpp src/http/DicomWeb.cpp \
       src/dicom/InstanceIndex.cpp src/dicom/DicomServer.cpp 2>/dev/null || true
