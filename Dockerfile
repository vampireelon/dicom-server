# syntax=docker/dockerfile:1
# Multi-stage build for dicom-server (Phase 11)

FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake git ca-certificates \
        libdcmtk-dev dcmtk \
        libsqlite3-dev \
        libboost-system-dev \
        libpq-dev \
        libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY config ./config
COPY tests ./tests

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DDICOMSERVER_BUILD_TESTS=OFF \
    && cmake --build build -j$(nproc)

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        libdcmtk17 \
        libsqlite3-0 \
        libboost-system1.83.0 \
        libpq5 \
        libssl3 \
        ca-certificates \
        curl \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -r -u 10001 -m -d /var/lib/dicom-server dicom

WORKDIR /var/lib/dicom-server

COPY --from=builder /src/build/dicom-server /usr/local/bin/dicom-server
COPY config/server.json /etc/dicom-server/server.json

RUN mkdir -p /var/lib/dicom-server/storage \
             /var/lib/dicom-server/worklist \
             /var/lib/dicom-server/data \
    && chown -R dicom:dicom /var/lib/dicom-server /etc/dicom-server

USER dicom

EXPOSE 4242 8042

ENV DICOMSERVER_CONFIG=/etc/dicom-server/server.json

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
  CMD curl -fsS http://127.0.0.1:8042/health || exit 1

ENTRYPOINT ["/usr/local/bin/dicom-server"]
CMD ["--config", "/etc/dicom-server/server.json"]
