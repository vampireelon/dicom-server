#!/bin/bash
# Reconstruct remaining C++ sources committed as base64 chunks
set -euo pipefail
cd "$(dirname "$0")"
cat .source_chunks/chunk_*.b64 | base64 -d | tar xzf -
echo "OK: remaining sources extracted into the working tree."
