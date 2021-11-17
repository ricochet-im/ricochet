option(ENABLE_LIBTEGO_TESTS "Build tests for libtego" ON)

include(lto)
include(compiler_opts)

if (ENABLE_LIBTEGO_TESTS)
    find_package(Catch2 REQUIRED)

    enable_testing()

    include(CTest)
    include(Catch)

    add_library(catch_tests STATIC main.cpp)
    setup_compiler(catch_tests)

    target_link_libraries(catch_tests PUBLIC Catch2::Catch2 tego)

    # add test sources here
    add_executable(libtego_tests test_init.cpp)
    setup_compiler(libtego_tests)

    target_link_libraries(libtego_tests PRIVATE catch_tests)

    catch_discover_tests(
        libtego_tests
        TEST_PREFIX
        "unittest."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "unittest."
        OUTPUT_SUFFIX
        ".xml")

endif ()
