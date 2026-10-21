#include <catch2/catch_test_macros.hpp>

#include "dicomserver/Logging.hpp"

TEST_CASE("Logging can be initialised and used", "[logging]") {
    dicomserver::Logging::Init("debug");
    dicomserver::Logging::Init("info");

    auto logger = dicomserver::Logging::Get();
    REQUIRE(logger != nullptr);

    DS_TRACE("trace message");
    DS_DEBUG("debug message");
    DS_INFO("info message from unit test");
    DS_WARN("warn message");
    DS_ERROR("error message (expected in test)");

    dicomserver::Logging::SetLevel("error");
    dicomserver::Logging::Shutdown();
}
