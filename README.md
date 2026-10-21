# DICOM Server (C++17+) v1.8

Orthanc-inspired DICOM SCP with REST, **DICOMweb**, plugins, multi-backend index, MPPS, security, and Web UI.

## Quick start

```bash
sudo apt install libdcmtk-dev dcmtk libsqlite3-dev libboost-system-dev libpq-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
./build/dicom-server --config config/server.json

# DICOM
echoscu -aec DICOMSERVER localhost 4242

# Web UI / REST
xdg-open http://localhost:8042/
```

## Phase 14 – DICOMweb (PS3.18 subset)

Base path: `/dicom-web`

| Service | Methods |
|---------|--------|
| **QIDO-RS** | `GET /dicom-web/studies[?PatientID=&StudyDate=&…]` |
| | `GET /dicom-web/studies/{study}/series` |
| | `GET /dicom-web/studies/{study}/series/{series}/instances` |
| **WADO-RS** | `GET …/instances/{instance}` → `application/dicom` |
| | `GET …/instances/{instance}/metadata` → `application/dicom+json` |
| | `GET /dicom-web/instances/{instance}` shortcut |
| **STOW-RS** | `POST /dicom-web/studies` body=`application/dicom` or `multipart/related` |

```bash
curl -X POST -H 'Content-Type: application/dicom' \
  --data-binary @image.dcm http://localhost:8042/dicom-web/studies
curl 'http://localhost:8042/dicom-web/studies?PatientID=DW001'
curl -o out.dcm http://localhost:8042/dicom-web/instances/<SOPInstanceUID>
```

## Feature matrix

| Phase | Feature |
|-------|--------|
| 0–6 | Core DIMSE, TLS, MWL, Metrics, Audit |
| 7 | HTTP REST |
| 8 | SQLite / PostgreSQL |
| 9 | Plugins |
| 10 | MPPS, SC N-EVENT-REPORT |
| 11 | Docker, SIGHUP, limits |
| 12 | AE ACL, rate limit, anonymize |
| 13 | Web UI |
| **14** | **DICOMweb QIDO / WADO / STOW** |

## Docker

```bash
docker compose up -d
# UI: http://localhost:8042/
```

---

License: MIT
