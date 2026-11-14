# Golden Fixture Tests
# Tests decode -> IR -> C -> compile -> execute -> verify pipeline

cmake_minimum_required(VERSION 3.25)

# Enable testing
enable_testing()

# Find GoogleTest via FetchContent
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(kirbyrecomp_golden_tests
    test_runner.cpp
)

target_include_directories(kirbyrecomp_golden_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/translator
    ${CMAKE_SOURCE_DIR}/src/codegen
    ${CMAKE_SOURCE_DIR}/src/rt
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_golden_tests PRIVATE
    kirbyrecomp_codegen
    sr_common
    sr_ir
    sr_translator
    sr_platform
    sr_runtime
    nlohmann_json::nlohmann_json
    Capstone::Capstone
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_golden_tests PRIVATE cxx_std_20)
target_compile_definitions(kirbyrecomp_golden_tests PRIVATE
    KIRBYRECOMP_BUILD_CODEGEN
    KIRBYRECOMP_BUILD_RT
)

# Let tests reach internal headers
target_include_directories(kirbyrecomp_golden_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Define fixture directory for tests
target_compile_definitions(kirbyrecomp_golden_tests PRIVATE
    GOLDEN_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)

# Add the main test suite
include(GoogleTest)
gtest_discover_tests(kirbyrecomp_golden_tests
    EXTRA_ARGS --gtest_filter=GoldenFixtures.*
)

# -----------------------------------------------------------------------------
# Per-fixture test targets (for individual test execution)
# -----------------------------------------------------------------------------

# Discover all fixture files and create a test for each
file(GLOB GOLDEN_FIXTURES "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/*.json")

foreach(fixture_file ${GOLDEN_FIXTURES})
    get_filename_component(fixture_name ${fixture_file} NAME_WE)

    # Create a custom target for each fixture that can be run individually
    add_custom_target(golden_${fixture_name}
        COMMAND $<TARGET_FILE:kirbyrecomp_golden_tests> --gtest_filter="GoldenFixtures.DiscoverFixtures/${fixture_name}*"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running golden fixture test: ${fixture_name}"
        VERBATIM
    )

    # Add to CTest as a named test
    add_test(
        NAME golden_${fixture_name}
        COMMAND $<TARGET_FILE:kirbyrecomp_golden_tests>
            --gtest_filter="GoldenFixtures.DiscoverFixtures/${fixture_name}*"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(golden_${fixture_name} PROPERTIES
        TIMEOUT 60
        LABELS "golden;${fixture_name}"
    )
endforeach()

# Convenience target to run all golden tests
add_custom_target(golden_tests_all
    DEPENDS ${ARGN}
)
set_property(GLOBAL PROPERTY GOLDEN_TEST_TARGETS ${GOLDEN_TEST_TARGETS})

# Add a summary test that runs all fixtures
add_test(
    NAME golden_all_fixtures
    COMMAND $<TARGET_FILE:kirbyrecomp_golden_tests>
        --gtest_filter="GoldenFixtures.*"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(golden_all_fixtures PROPERTIES
    TIMEOUT 300
    LABELS "golden;all"
)