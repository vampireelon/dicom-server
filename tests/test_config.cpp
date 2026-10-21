#include <catch2/catch_test_macros.hpp>

#include "dicomserver/Config.hpp"
#include "dicomserver/Exception.hpp"

#include <nlohmann/json.hpp>

using namespace dicomserver;

TEST_CASE("Config loads defaults when keys are missing", "[config]") {
    Config::Reset();
    nlohmann::json j = nlohmann::json::object();
    Config::LoadFromJson(j);

    const auto& cfg = Config::Get();
    REQUIRE(cfg.aet == "DICOMSERVER");
    REQUIRE(cfg.port == 4242);
    REQUIRE(cfg.workerThreads == 4);
    REQUIRE(cfg.modalities.empty());
}

TEST_CASE("Config parses full JSON", "[config]") {
    Config::Reset();
    nlohmann::json j = {
        {"DicomAet", "MYAET"},
        {"DicomPort", 11112},
        {"DicomThreads", 8},
        {"LogLevel", "debug"},
        {"StorageDirectory", "/tmp/dicom"},
        {"DicomModalities", {
            {"WS1", {"WS1", "10.0.0.5", 104}}
        }}
    };
    Config::LoadFromJson(j);

    const auto& cfg = Config::Get();
    REQUIRE(cfg.aet == "MYAET");
    REQUIRE(cfg.port == 11112);
    REQUIRE(cfg.workerThreads == 8);
    REQUIRE(cfg.logLevel == "debug");
    REQUIRE(cfg.storageDirectory == "/tmp/dicom");
    REQUIRE(cfg.modalities.size() == 1);
    REQUIRE(cfg.modalities[0].aet == "WS1");
    REQUIRE(cfg.modalities[0].host == "10.0.0.5");
    REQUIRE(cfg.modalities[0].port == 104);
}

TEST_CASE("Config rejects invalid AET length", "[config]") {
    Config::Reset();
    nlohmann::json j = {{"DicomAet", ""}};
    REQUIRE_THROWS_AS(Config::LoadFromJson(j), Exception);

    Config::Reset();
    j = {{"DicomAet", "THIS_AET_IS_WAY_TOO_LONG"}};
    REQUIRE_THROWS_AS(Config::LoadFromJson(j), Exception);
}

TEST_CASE("Config rejects zero port / threads", "[config]") {
    Config::Reset();
    REQUIRE_THROWS_AS(
        Config::LoadFromJson({{"DicomPort", 0}}), Exception);

    Config::Reset();
    REQUIRE_THROWS_AS(
        Config::LoadFromJson({{"DicomThreads", 0}}), Exception);
}

TEST_CASE("Config::Get before Load throws", "[config]") {
    Config::Reset();
    REQUIRE_THROWS_AS(Config::Get(), Exception);
}
