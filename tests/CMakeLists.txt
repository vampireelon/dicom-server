add_executable(dicomserver_tests
    test_exception.cpp
    test_config.cpp
    test_logging.cpp
)

target_link_libraries(dicomserver_tests
    PRIVATE
        dicomserver_core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(dicomserver_tests)
