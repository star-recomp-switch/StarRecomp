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
# Set TMP/TEMP to build directory for tests that invoke g++ (avoids C:\Windows permission issues)
set_tests_properties(kirbyrecomp_nso_integration_all PROPERTIES
    ENVIRONMENT "TMP=${CMAKE_CURRENT_BINARY_DIR};TEMP=${CMAKE_CURRENT_BINARY_DIR}"
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
# Set TMP/TEMP to build directory for tests that invoke g++ (avoids C:\Windows permission issues)
set_tests_properties(kirbyrecomp_hle_all PROPERTIES
    ENVIRONMENT "TMP=${CMAKE_CURRENT_BINARY_DIR};TEMP=${CMAKE_CURRENT_BINARY_DIR}"
)

# Test target: ASTC Decoder tests
add_executable(kirbyrecomp_astc_tests
    test_astc.cpp
)

# Test target: ASTC to BC7 Transcoding tests
add_executable(kirbyrecomp_astc_bc7_tests
    test_astc_bc7.cpp
)

target_include_directories(kirbyrecomp_astc_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/nvn
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_astc_tests PRIVATE
    kirbyrecomp_nvn
    VMA::VMA
    Vulkan::Vulkan
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_astc_tests PRIVATE cxx_std_20)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_astc_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_astc_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
add_test(
    NAME kirbyrecomp_astc_all
    COMMAND $<TARGET_FILE:kirbyrecomp_astc_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(kirbyrecomp_astc_all PROPERTIES
    TIMEOUT 120
    LABELS "astc;integration"
)
# Set TMP/TEMP to build directory for tests that invoke g++ (avoids C:\Windows permission issues)
set_tests_properties(kirbyrecomp_astc_all PROPERTIES
    ENVIRONMENT "TMP=${CMAKE_CURRENT_BINARY_DIR};TEMP=${CMAKE_CURRENT_BINARY_DIR}"
)

# Test target: ASTC to BC7 Transcoding tests
target_include_directories(kirbyrecomp_astc_bc7_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/nvn
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_astc_bc7_tests PRIVATE
    kirbyrecomp_nvn
    VMA::VMA
    Vulkan::Vulkan
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_astc_bc7_tests PRIVATE cxx_std_20)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_astc_bc7_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_astc_bc7_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
add_test(
    NAME kirbyrecomp_astc_bc7_all
    COMMAND $<TARGET_FILE:kirbyrecomp_astc_bc7_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(kirbyrecomp_astc_bc7_all PROPERTIES
    TIMEOUT 120
    LABELS "astc;bc7;integration"
)
# Set TMP/TEMP to build directory for tests that invoke g++ (avoids C:\Windows permission issues)
set_tests_properties(kirbyrecomp_astc_bc7_all PROPERTIES
    ENVIRONMENT "TMP=${CMAKE_CURRENT_BINARY_DIR};TEMP=${CMAKE_CURRENT_BINARY_DIR}"
)

# Test target: AArch64 Decoder tests
add_executable(kirbyrecomp_decoder_tests
    ../unit/test_decoder.cpp
)

target_include_directories(kirbyrecomp_decoder_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/translator
    ${CMAKE_SOURCE_DIR}/src/translator/decoder
    ${SR_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../unit
)

target_link_libraries(kirbyrecomp_decoder_tests PRIVATE
    kirbyrecomp_translator
    sr_common
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_decoder_tests PRIVATE cxx_std_20)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_decoder_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_decoder_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
add_test(
    NAME kirbyrecomp_decoder_all
    COMMAND $<TARGET_FILE:kirbyrecomp_decoder_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(kirbyrecomp_decoder_all PROPERTIES
    TIMEOUT 120
    LABELS "decoder;unit"
)
# Set TMP/TEMP to build directory for tests that invoke g++ (avoids C:\Windows permission issues)
set_tests_properties(kirbyrecomp_decoder_all PROPERTIES
    ENVIRONMENT "TMP=${CMAKE_CURRENT_BINARY_DIR};TEMP=${CMAKE_CURRENT_BINARY_DIR}"
)

# Test target: Shader Translation tests
add_executable(kirbyrecomp_shader_tests
    test_shader.cpp
)

target_include_directories(kirbyrecomp_shader_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/shader
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_shader_tests PRIVATE
    kirbyrecomp_shader
    kirbyrecomp_rt
    sr_common
    GTest::gtest_main
)

target_compile_features(kirbyrecomp_shader_tests PRIVATE cxx_std_20)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_shader_tests SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Let tests reach internal headers
target_include_directories(kirbyrecomp_shader_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
add_test(
    NAME kirbyrecomp_shader_all
    COMMAND $<TARGET_FILE:kirbyrecomp_shader_tests>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(kirbyrecomp_shader_all PROPERTIES
    TIMEOUT 120
    LABELS "shader;integration"
)
# Set TMP/TEMP to build directory for tests that invoke g++ (avoids C:\Windows permission issues)
set_tests_properties(kirbyrecomp_shader_all PROPERTIES
    ENVIRONMENT "TMP=${CMAKE_CURRENT_BINARY_DIR};TEMP=${CMAKE_CURRENT_BINARY_DIR}"
)