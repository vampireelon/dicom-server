#include <catch2/catch_test_macros.hpp>

#include "dicomserver/Exception.hpp"

using namespace dicomserver;

TEST_CASE("ErrorCode ToString covers all values", "[exception]") {
    REQUIRE(ToString(ErrorCode::Success) == "Success");
    REQUIRE(ToString(ErrorCode::InternalError) == "InternalError");
    REQUIRE(ToString(ErrorCode::NotImplemented) == "NotImplemented");
    REQUIRE(ToString(ErrorCode::ConfigParseError) == "ConfigParseError");
    REQUIRE(ToString(ErrorCode::DicomPortInUse) == "DicomPortInUse");
    REQUIRE(ToString(ErrorCode::Unknown) == "Unknown");
}

TEST_CASE("Exception carries ErrorCode and message", "[exception]") {
    Exception ex(ErrorCode::ConfigMissingKey, "key 'foo' not found");
    REQUIRE(ex.Code() == ErrorCode::ConfigMissingKey);
    REQUIRE(std::string(ex.what()).find("ConfigMissingKey") != std::string::npos);
    REQUIRE(std::string(ex.what()).find("key 'foo' not found") != std::string::npos);
}

TEST_CASE("Exception helper factories", "[exception]") {
    auto e1 = Exception::Internal("boom");
    REQUIRE(e1.Code() == ErrorCode::InternalError);

    auto e2 = Exception::NotImplemented("C-STORE");
    REQUIRE(e2.Code() == ErrorCode::NotImplemented);
    REQUIRE(std::string(e2.what()).find("C-STORE") != std::string::npos);

    auto e3 = Exception::BadSequence("must call Start() first");
    REQUIRE(e3.Code() == ErrorCode::BadSequenceOfCalls);
}
