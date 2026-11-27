# BFRES Parser Module
# Parses Nintendo BFRES files to extract texture data

cmake_minimum_required(VERSION 3.20)

project(kirbyrecomp_bfres)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Zstd
find_package(Zstd REQUIRED)

# BFRES parser library
add_library(kirbyrecomp_bfres STATIC
    bfres.cpp
    cmp_decompress.cpp
)

target_include_directories(kirbyrecomp_bfres PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_bfres SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

target_link_libraries(kirbyrecomp_bfres PUBLIC
    zstd::libzstd_static
)

# Install headers
install(FILES
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/bfres/bfres.h
    DESTINATION include/kirbyrecomp/bfres
)

# CMP decompress tool
add_executable(cmp_decompress
    cmp_decompress.cpp
)
target_compile_definitions(cmp_decompress PRIVATE CMP_DECOMPRESS_MAIN)
target_link_libraries(cmp_decompress PRIVATE
    kirbyrecomp_bfres
    zstd::libzstd_static
)

# Real BFRES test tool
add_executable(bfres_real_test
    test_real_bfres.cpp
)
target_link_libraries(bfres_real_test PRIVATE
    kirbyrecomp_bfres
    kirbyrecomp_nvn
    zstd::libzstd_static
)

# BFRES NVN integration test tool
add_executable(bfres_nvn_test
    test_bfres_nvn.cpp
)
target_link_libraries(bfres_nvn_test PRIVATE
    kirbyrecomp_bfres
    kirbyrecomp_nvn
    zstd::libzstd_static
    Vulkan::Vulkan
)

# Tests
enable_testing()

add_executable(bfres_test
    test_bfres.cpp
)

target_link_libraries(bfres_test PRIVATE
    kirbyrecomp_bfres
    kirbyrecomp_nvn
    GTest::gtest_main
)

target_include_directories(bfres_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(bfres_test SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

include(GoogleTest)
gtest_discover_tests(bfres_test)