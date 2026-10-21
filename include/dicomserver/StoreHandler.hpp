#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace dicomserver {

enum class StoreStatus : uint16_t {
    Success                         = 0x0000,
    RefusedOutOfResources           = 0xA700,
    ErrorDataSetDoesNotMatchSOPClass = 0xA900,
    ErrorCannotUnderstand           = 0xC000,
    WarningCoercionOfDataElements   = 0xB000,
};

struct StoreInstanceInfo {
    std::string sopClassUid;
    std::string sopInstanceUid;
    std::string studyInstanceUid;
    std::string seriesInstanceUid;
    std::string patientId;
    std::string patientName;
    std::string transferSyntaxUid;
    std::string callingAet;
    std::string filePath;
};

class IStoreRequestHandler {
public:
    virtual ~IStoreRequestHandler() = default;
    virtual StoreStatus HandleStore(StoreInstanceInfo& info) = 0;
};

class IStoreRequestHandlerFactory {
public:
    virtual ~IStoreRequestHandlerFactory() = default;
    virtual std::unique_ptr<IStoreRequestHandler> Create() = 0;
};

class FileStoreRequestHandler : public IStoreRequestHandler {
public:
    explicit FileStoreRequestHandler(std::string storageDir);
    StoreStatus HandleStore(StoreInstanceInfo& info) override;
private:
    std::string storageDir_;
};

class FileStoreRequestHandlerFactory : public IStoreRequestHandlerFactory {
public:
    explicit FileStoreRequestHandlerFactory(std::string storageDir)
        : storageDir_(std::move(storageDir)) {}
    std::unique_ptr<IStoreRequestHandler> Create() override {
        return std::make_unique<FileStoreRequestHandler>(storageDir_);
    }
private:
    std::string storageDir_;
};

}  // namespace dicomserver
