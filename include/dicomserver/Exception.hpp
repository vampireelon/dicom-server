#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace dicomserver {

enum class ErrorCode {
    Success = 0,
    InternalError,
    NotImplemented,
    ParameterOutOfRange,
    BadSequenceOfCalls,
    NullPointer,
    ConfigParseError,
    ConfigMissingKey,
    ConfigInvalidValue,
    DicomPortInUse,
    DicomNetworkError,
    DicomAssociationRejected,
    DicomProtocolError,
    StorageError,
    StorageFull,
    Unknown = 9999
};

[[nodiscard]] std::string_view ToString(ErrorCode code) noexcept;

class Exception : public std::runtime_error {
public:
    explicit Exception(ErrorCode code, std::string message = {});
    [[nodiscard]] ErrorCode Code() const noexcept { return code_; }
    [[nodiscard]] static Exception Internal(std::string msg);
    [[nodiscard]] static Exception NotImplemented(std::string_view feature);
    [[nodiscard]] static Exception BadSequence(std::string_view what);
private:
    ErrorCode code_;
};

}  // namespace dicomserver
