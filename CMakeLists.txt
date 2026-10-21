cmake_minimum_required(VERSION 3.16)
project(dicom-server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DICOM_SERVER_BUILD_TESTS "Build unit tests" ON)
option(DICOM_SERVER_USE_POSTGRES "Enable PostgreSQL index backend" ON)

find_package(DCMTK REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

if(DICOM_SERVER_USE_POSTGRES)
  find_package(PostgreSQL)
endif()

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.12.0)
FetchContent_MakeAvailable(json spdlog)

if(DICOM_SERVER_BUILD_TESTS)
  FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.4.0)
  FetchContent_MakeAvailable(Catch2)
endif()

set(DICOM_SERVER_SOURCES
  src/main.cpp
  src/core/Config.cpp
  src/core/Exception.cpp
  src/core/Logging.cpp
  src/core/Metrics.cpp
  src/core/AccessControl.cpp
  src/core/Audit.cpp
  src/core/PluginManager.cpp
  src/dicom/DicomServer.cpp
  src/dicom/DicomScu.cpp
  src/dicom/InstanceIndex.cpp
  src/dicom/StoreHandler.cpp
  src/dicom/WorklistIndex.cpp
  src/dicom/MppsIndex.cpp
  src/http/HttpServer.cpp
  src/http/WebUi.cpp
  src/http/DicomWeb.cpp
)

add_executable(dicom-server ${DICOM_SERVER_SOURCES})

target_include_directories(dicom-server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${DCMTK_INCLUDE_DIRS}
)

target_link_libraries(dicom-server PRIVATE
  ${DCMTK_LIBRARIES}
  Boost::system
  Threads::Threads
  SQLite::SQLite3
  nlohmann_json::nlohmann_json
  spdlog::spdlog
)

if(DICOM_SERVER_USE_POSTGRES AND PostgreSQL_FOUND)
  target_compile_definitions(dicom-server PRIVATE DICOM_SERVER_HAS_POSTGRES=1)
  target_include_directories(dicom-server PRIVATE ${PostgreSQL_INCLUDE_DIRS})
  target_link_libraries(dicom-server PRIVATE ${PostgreSQL_LIBRARIES})
endif()

if(DICOM_SERVER_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

install(TARGETS dicom-server RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION etc/dicom-server)
