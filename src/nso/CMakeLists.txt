# KirbyRecomp NSO Parser Library

cmake_minimum_required(VERSION 3.25)

# Target: kirbyrecomp_nso
add_library(kirbyrecomp_nso STATIC)

target_sources(kirbyrecomp_nso PRIVATE
    nso_parser.cpp
)

target_include_directories(kirbyrecomp_nso PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(kirbyrecomp_nso PUBLIC
    sr_common
)

target_compile_features(kirbyrecomp_nso PUBLIC cxx_std_20)
target_compile_definitions(kirbyrecomp_nso PUBLIC KIRBYRECOMP_BUILD_NSO)

# Link OpenSSL for SHA-256
find_package(OpenSSL REQUIRED)

# Link LZ4 for decompression (built from vendor/lz4)
target_link_libraries(kirbyrecomp_nso PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    lz4
)

# Add mingw64 C++ standard library includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_nso SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Install headers
install(FILES
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/nso/nso.h
    DESTINATION include/kirbyrecomp/nso
)

install(TARGETS kirbyrecomp_nso
    EXPORT KirbyRecompTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)