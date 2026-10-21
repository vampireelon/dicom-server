#include "dicomserver/Exception.hpp"

#include <sstream>

namespace dicomserver {

std::string_view ToString(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Success:                  return "Success";
        case ErrorCode::InternalError:            return "InternalError";
        case ErrorCode::NotImplemented:           return "NotImplemented";
        case ErrorCode::ParameterOutOfRange:      return "ParameterOutOfRange";
        case ErrorCode::BadSequenceOfCalls:       return "BadSequenceOfCalls";
        case ErrorCode::NullPointer:              return "NullPointer";
        case ErrorCode::ConfigParseError:         return "ConfigParseError";
        case ErrorCode::ConfigMissingKey:         return "ConfigMissingKey";
        case ErrorCode::ConfigInvalidValue:       return "ConfigInvalidValue";
        case ErrorCode::DicomPortInUse:           return "DicomPortInUse";
        case ErrorCode::DicomNetworkError:        return "DicomNetworkError";
        case ErrorCode::DicomAssociationRejected: return "DicomAssociationRejected";
        case ErrorCode::DicomProtocolError:       return "DicomProtocolError";
        case ErrorCode::StorageError:             return "StorageError";
        case ErrorCode::StorageFull:              return "StorageFull";
        case ErrorCode::Unknown:                  return "Unknown";
    }
    return "Unknown";
}

Exception::Exception(ErrorCode code, std::string message)
    : std::runtime_error([&]() {
          std::ostringstream oss;
          oss << '[' << ToString(code) << ']';
          if (!message.empty()) {
              oss << ' ' << message;
          }
          return oss.str();
      }()),
      code_(code) {}

Exception Exception::Internal(std::string msg) {
    return Exception(ErrorCode::InternalError, std::move(msg));
}

Exception Exception::NotImplemented(std::string_view feature) {
    return Exception(ErrorCode::NotImplemented,
                     "Feature not implemented: " + std::string(feature));
}

Exception Exception::BadSequence(std::string_view what) {
    return Exception(ErrorCode::BadSequenceOfCalls, std::string(what));
}

}  // namespace dicomserver
