# Integration Tests for KirbyRecomp

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

# Test target: NSO Parser integration tests (without std::filesystem)
add_executable(kirbyrecomp_nso_integration_tests
    test_nso.cpp
)

target_include_directories(kirbyrecomp_nso_integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_nso_integration_tests PRIVATE
    kirbyrecomp_nso
    sr_common
    capstone::capstone_static
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_nso_integration_tests PRIVATE cxx_std_20)
target_compile_definitions(kirbyrecomp_nso_integration_tests PRIVATE
    KIRBYRECOMP_BUILD_NSO
    SR_INCLUDE_DIR="${SR_INCLUDE_DIR}"
)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_nso_integration_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_nso_integration_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
# Don't run gtest_discover_tests at configure time (needs PATH for DLLs)
# Instead, add the test executable directly to CTest
add_test(
    NAME kirbyrecomp_nso_integration_all
    COMMAND $<TARGET_FILE:kirbyrecomp_nso_integration_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(kirbyrecomp_nso_integration_all PROPERTIES
    TIMEOUT 120
    LABELS "nso;integration"
)

# Test target: HLE Services integration tests
add_executable(kirbyrecomp_hle_tests
    test_hle.cpp
)

target_include_directories(kirbyrecomp_hle_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_hle_tests PRIVATE
    kirbyrecomp_hle
    kirbyrecomp_rt
    sr_common
    Vulkan::Vulkan
    Opus::opus
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_hle_tests PRIVATE cxx_std_20)
target_compile_definitions(kirbyrecomp_hle_tests PRIVATE
    KIRBYRECOMP_BUILD_HLE
    SR_INCLUDE_DIR="${SR_INCLUDE_DIR}"
)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_hle_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_hle_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Add to CTest - HLE tests
add_test(
    NAME kirbyrecomp_hle_all
    COMMAND $<TARGET_FILE:kirbyrecomp_hle_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(kirbyrecomp_hle_all PROPERTIES
    TIMEOUT 120
    LABELS "hle;integration"
)