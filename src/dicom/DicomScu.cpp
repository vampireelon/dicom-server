#include "dicomserver/DicomScu.hpp"
#include "dicomserver/Logging.hpp"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/ofstd/ofstd.h>
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>

#include <cstring>

namespace dicomserver {

namespace {

std::string OfCondText(const OFCondition& cond) {
    return cond.text() ? cond.text() : "unknown DCMTK error";
}

const char* kStorageTS[] = {
    UID_LittleEndianExplicitTransferSyntax,
    UID_LittleEndianImplicitTransferSyntax,
    UID_BigEndianExplicitTransferSyntax,
    nullptr
};

void AddStorageContexts(T_ASC_Parameters* params,
                        const std::vector<IndexedInstance>& instances) {
    std::vector<std::string> sopClasses;
    for (const auto& inst : instances) {
        if (inst.sopClassUid.empty()) continue;
        bool found = false;
        for (const auto& s : sopClasses) {
            if (s == inst.sopClassUid) { found = true; break; }
        }
        if (!found) sopClasses.push_back(inst.sopClassUid);
    }
    sopClasses.push_back(UID_SecondaryCaptureImageStorage);

    int pcid = 1;
    for (const auto& sc : sopClasses) {
        const char* ts[4];
        int n = 0;
        while (kStorageTS[n]) { ts[n] = kStorageTS[n]; ++n; }
        ASC_addPresentationContext(params, static_cast<T_ASC_PresentationContextID>(pcid),
                                   sc.c_str(), ts, n);
        pcid += 2;
        if (pcid > 255) break;
    }
}

StoreScuResult SendFiles(T_ASC_Association* assoc,
                         const std::vector<IndexedInstance>& instances,
                         uint32_t timeoutSec) {
    StoreScuResult result;
    DIC_US msgId = 1;

    for (const auto& inst : instances) {
        if (inst.filePath.empty()) {
            DS_WARN("C-STORE SCU skip – empty path for SOP {}", inst.sopInstanceUid);
            ++result.failed;
            continue;
        }

        DcmFileFormat ff;
        OFCondition cond = ff.loadFile(inst.filePath.c_str());
        if (cond.bad()) {
            DS_ERROR("Cannot load {}: {}", inst.filePath, OfCondText(cond));
            ++result.failed;
            continue;
        }

        DcmDataset* dset = ff.getDataset();
        OFString sopClass, sopInstance, ts;
        dset->findAndGetOFString(DCM_SOPClassUID, sopClass);
        dset->findAndGetOFString(DCM_SOPInstanceUID, sopInstance);
        E_TransferSyntax xfer = dset->getOriginalXfer();
        if (xfer == EXS_Unknown) xfer = EXS_LittleEndianExplicit;
        DcmXfer xferObj(xfer);
        ts = xferObj.getXferID();

        T_ASC_PresentationContextID presId =
            ASC_findAcceptedPresentationContextID(assoc, sopClass.c_str());
        if (presId == 0) {
            DS_ERROR("No accepted presentation context for SOP Class {}",
                     sopClass.c_str());
            ++result.failed;
            continue;
        }

        T_DIMSE_C_StoreRQ req = {};
        req.MessageID = msgId++;
        std::strncpy(req.AffectedSOPClassUID, sopClass.c_str(),
                     sizeof(req.AffectedSOPClassUID) - 1);
        std::strncpy(req.AffectedSOPInstanceUID, sopInstance.c_str(),
                     sizeof(req.AffectedSOPInstanceUID) - 1);
        req.DataSetType = DIMSE_DATASET_PRESENT;
        req.Priority = DIMSE_PRIORITY_MEDIUM;

        T_DIMSE_C_StoreRSP rsp = {};
        DcmDataset* statusDetail = nullptr;

        cond = DIMSE_storeUser(
            assoc, presId, &req,
            inst.filePath.c_str(), /*imageDataSet=*/nullptr,
            /*callback=*/nullptr, /*callbackData=*/nullptr,
            DIMSE_BLOCKING, static_cast<int>(timeoutSec),
            &rsp, &statusDetail);

        delete statusDetail;

        if (cond.bad()) {
            DS_ERROR("DIMSE_storeUser failed for {}: {}",
                     inst.sopInstanceUid, OfCondText(cond));
            ++result.failed;
        } else if (rsp.DimseStatus == STATUS_Success ||
                   rsp.DimseStatus == 0x0000) {
            DS_INFO("C-STORE SCU OK  SOP={}", inst.sopInstanceUid);
            ++result.completed;
        } else if ((rsp.DimseStatus & 0xF000) == 0xB000) {
            DS_WARN("C-STORE SCU warning 0x{:04X}  SOP={}",
                    rsp.DimseStatus, inst.sopInstanceUid);
            ++result.warning;
            ++result.completed;
        } else {
            DS_ERROR("C-STORE SCU status 0x{:04X}  SOP={}",
                     rsp.DimseStatus, inst.sopInstanceUid);
            ++result.failed;
        }
    }
    return result;
}

}  // namespace

const ServerConfig::RemoteModality* FindModality(
    const std::vector<ServerConfig::RemoteModality>& modalities,
    const std::string& aet) {
    for (const auto& m : modalities) {
        if (m.aet == aet) return &m;
    }
    return nullptr;
}

StoreScuResult StoreInstancesToRemote(
    const std::string& ourAet,
    const ServerConfig::RemoteModality& dest,
    const std::vector<IndexedInstance>& instances,
    uint32_t timeoutSec) {

    StoreScuResult result;
    if (instances.empty()) return result;

    T_ASC_Network* net = nullptr;
    OFCondition cond = ASC_initializeNetwork(NET_REQUESTOR, 0, 30, &net);
    if (cond.bad()) {
        DS_ERROR("ASC_initializeNetwork (SCU) failed: {}", OfCondText(cond));
        result.failed = static_cast<unsigned>(instances.size());
        return result;
    }

    T_ASC_Parameters* params = nullptr;
    ASC_createAssociationParameters(&params, ASC_DEFAULTMAXPDU);
    ASC_setAPTitles(params, ourAet.c_str(), dest.aet.c_str(), nullptr);

    std::string peer = dest.host + ":" + std::to_string(dest.port);
    ASC_setPresentationAddresses(params, "localhost", peer.c_str());

    AddStorageContexts(params, instances);

    T_ASC_Association* assoc = nullptr;
    DS_INFO("C-MOVE sub-op: connecting to {} @ {}:{}", dest.aet, dest.host, dest.port);
    cond = ASC_requestAssociation(net, params, &assoc);
    if (cond.bad()) {
        DS_ERROR("ASC_requestAssociation to {} failed: {}", dest.aet, OfCondText(cond));
        ASC_destroyAssociationParameters(&params);
        ASC_dropNetwork(&net);
        result.failed = static_cast<unsigned>(instances.size());
        return result;
    }

    result = SendFiles(assoc, instances, timeoutSec);

    ASC_releaseAssociation(assoc);
    ASC_destroyAssociation(&assoc);
    ASC_dropNetwork(&net);
    return result;
}

StoreScuResult StoreInstancesOnAssociation(
    void* assocPtr,
    const std::vector<IndexedInstance>& instances,
    uint32_t timeoutSec) {
    auto* assoc = static_cast<T_ASC_Association*>(assocPtr);
    if (!assoc) {
        StoreScuResult r;
        r.failed = static_cast<unsigned>(instances.size());
        return r;
    }
    return SendFiles(assoc, instances, timeoutSec);
}

}  // namespace dicomserver
