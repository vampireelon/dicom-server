cmake_minimum_required(VERSION 3.16)
project(DicomServer
    VERSION 1.8.0
    DESCRIPTION "Lightweight DICOM Server (C++17+) inspired by Orthanc"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DICOMSERVER_BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(spdlog)

if(DICOMSERVER_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

find_package(DCMTK REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)
message(STATUS "PostgreSQL: ${PostgreSQL_LIBRARIES}")
message(STATUS "Boost: ${Boost_VERSION}")
message(STATUS "SQLite3: ${SQLite3_LIBRARIES}")
message(STATUS "DCMTK include dirs : ${DCMTK_INCLUDE_DIRS}")
message(STATUS "DCMTK libraries    : ${DCMTK_LIBRARIES}")

add_library(dicomserver_core
    src/core/Exception.cpp
    src/core/Logging.cpp
    src/core/Config.cpp
    src/core/Metrics.cpp
    src/core/Audit.cpp
    src/core/PluginManager.cpp
    src/core/AccessControl.cpp
    src/dicom/DicomServer.cpp
    src/dicom/StoreHandler.cpp
    src/dicom/InstanceIndex.cpp
    src/dicom/DicomScu.cpp
    src/dicom/WorklistIndex.cpp
    src/dicom/MppsIndex.cpp
    src/http/HttpServer.cpp
    src/http/WebUi.cpp
    src/http/DicomWeb.cpp
)

target_include_directories(dicomserver_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${DCMTK_INCLUDE_DIRS}
)

target_link_libraries(dicomserver_core
    PUBLIC
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        ${DCMTK_LIBRARIES}
        SQLite::SQLite3
        Boost::system
        PostgreSQL::PostgreSQL
        ${CMAKE_DL_LIBS}
)

target_compile_definitions(dicomserver_core PUBLIC DICOMSERVER_HAS_DCMTK=1)

if(MSVC)
    target_compile_options(dicomserver_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(dicomserver_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(dicom-server
    src/main.cpp
)

target_link_libraries(dicom-server
    PRIVATE
        dicomserver_core
)

if(DICOMSERVER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS dicom-server RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION etc/dicom-server)

add_library(example_logger SHARED plugins/example_logger.cpp)
target_include_directories(example_logger PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(example_logger PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "example_logger"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
