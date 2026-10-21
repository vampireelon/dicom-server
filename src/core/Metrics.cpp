#include "dicomserver/Metrics.hpp"

#include <sstream>

namespace dicomserver {

Metrics& Metrics::Instance() {
    static Metrics m;
    return m;
}

std::string Metrics::Snapshot() const {
    std::ostringstream oss;
    oss << "associations{accepted=" << associationsAccepted.load()
        << " rejected=" << associationsRejected.load()
        << " active=" << associationsActive.load() << "}"
        << " echo=" << cEcho.load()
        << " store{ok=" << cStoreSuccess.load()
        << " fail=" << cStoreFailed.load() << "}"
        << " find=" << cFind.load()
        << " mwl=" << mwlFind.load()
        << " move{ok=" << cMoveSuccess.load()
        << " fail=" << cMoveFailed.load() << "}"
        << " get{ok=" << cGetSuccess.load()
        << " fail=" << cGetFailed.load() << "}"
        << " naction=" << nAction.load()
        << " errors{dimse=" << dimseErrors.load()
        << " storage=" << storageErrors.load() << "}";
    return oss.str();
}

void Metrics::Reset() {
    associationsAccepted = 0;
    associationsRejected = 0;
    associationsActive = 0;
    cEcho = 0;
    cStoreSuccess = 0;
    cStoreFailed = 0;
    cFind = 0;
    cMoveSuccess = 0;
    cMoveFailed = 0;
    cGetSuccess = 0;
    cGetFailed = 0;
    nAction = 0;
    mwlFind = 0;
    dimseErrors = 0;
    storageErrors = 0;
}

}  // namespace dicomserver
