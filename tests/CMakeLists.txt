# Test build configuration for KirbyRecomp
# Integrates SwitchRecomp's test patterns (ctest, golden fixtures, end-to-end)

cmake_minimum_required(VERSION 3.25)

# Enable testing
enable_testing()

# Find GoogleTest via FetchContent (mirrors vendor/switchrecomp/tests/CMakeLists.txt)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories
set(KIRBYRECOMP_TEST_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/golden/fixtures")
set(SR_TEST_FIXTURES_DIR "${CMAKE_SOURCE_DIR}/vendor/switchrecomp/tests/fixtures")
set(SR_INCLUDE_DIR "${SR_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Test target 1: NSO Parser tests (WORKING)
# Tests parsing of real NSO files from game dumps
# -----------------------------------------------------------------------------
add_executable(kirbyrecomp_nso_tests
    integration/test_nso.cpp
)

target_include_directories(kirbyrecomp_nso_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_nso_tests PRIVATE
    kirbyrecomp_nso
    sr_common
    Capstone::Capstone
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_nso_tests PRIVATE cxx_std_20)
target_compile_definitions(kirbyrecomp_nso_tests PRIVATE
    KIRBYRECOMP_BUILD_NSO
    SR_INCLUDE_DIR="${SR_INCLUDE_DIR}"
)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_nso_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_nso_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
gtest_discover_tests(kirbyrecomp_nso_tests)


# -----------------------------------------------------------------------------
# Test target 2: End-to-end integer instruction tests (DISABLED - Capstone version issues)
# Runs SwitchRecomp's integer instruction tests against our full pipeline
# (decode -> IR -> C -> compile -> execute -> verify)
# -----------------------------------------------------------------------------
# add_executable(kirbyrecomp_e2e_integer_tests
#     integration/test_e2e_integer.cpp
# )

# target_include_directories(kirbyrecomp_e2e_integer_tests PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/src/translator
#     ${CMAKE_SOURCE_DIR}/src/translator/decoder
#     ${CMAKE_SOURCE_DIR}/src/codegen
#     ${CMAKE_SOURCE_DIR}/src/rt
#     ${SR_INCLUDE_DIR}
# )

# target_link_libraries(kirbyrecomp_e2e_integer_tests PRIVATE
#     kirbyrecomp_codegen
#     kirbyrecomp_rt
#     sr_common
#     sr_ir
#     sr_translator
#     sr_codegen
#     sr_platform
#     sr_runtime
#     nlohmann_json::nlohmann_json
#     Capstone::Capstone
#     GTest::gtest_main
# )

# target_compile_features(kirbyrecomp_e2e_integer_tests PRIVATE cxx_std_20)
# target_compile_definitions(kirbyrecomp_e2e_integer_tests PRIVATE
#     KIRBYRECOMP_BUILD_CODEGEN
#     KIRBYRECOMP_BUILD_RT
#     SR_TEST_FIXTURES_DIR="${SR_TEST_FIXTURES_DIR}"
#     SR_INCLUDE_DIR="${SR_INCLUDE_DIR}"
# )

# Add mingw64 C++ stdlib includes for clang
# if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
#     target_include_directories(kirbyrecomp_e2e_integer_tests SYSTEM PRIVATE
#         ${MINGW64_CXX_INCLUDE_DIR}
#         ${MINGW64_CXX_ARCH_INCLUDE_DIR}
#     )
# endif()

# # Let tests reach internal headers
# target_include_directories(kirbyrecomp_e2e_integer_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# include(GoogleTest)
# gtest_discover_tests(kirbyrecomp_e2e_integer_tests)


# -----------------------------------------------------------------------------
# Convenience target to run all tests
# -----------------------------------------------------------------------------
add_custom_target(kirbyrecomp_all_tests
    DEPENDS kirbyrecomp_nso_tests
)

# Add the main test suites to CTest
add_test(NAME kirbyrecomp_nso_all COMMAND $<TARGET_FILE:kirbyrecomp_nso_tests>)

set_tests_properties(kirbyrecomp_nso_all PROPERTIES TIMEOUT 120 LABELS "nso;integration")

# Include integration tests subdirectory
add_subdirectory(integration)