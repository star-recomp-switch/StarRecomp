# KFL (Kirby and the Forgotten Land) Module Loader
# Handles KFL-specific NSO modules and ExeFS parsing

cmake_minimum_required(VERSION 3.20)

project(kirbyrecomp_kfl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# KFL runtime library
add_library(kirbyrecomp_kfl STATIC
    kfl_nso.cpp
    kfl_save.cpp
)

target_include_directories(kirbyrecomp_kfl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(kirbyrecomp_kfl PUBLIC
    kirbyrecomp_nso
    kirbyrecomp_shader
    kirbyrecomp_nvn
    OpenSSL::SSL
    OpenSSL::Crypto
    LZ4::LZ4
    opus
)

# Add mingw64 C++ standard library includes for clang
add_mingw64_cxx_includes(kirbyrecomp_kfl)

# Compiler options
target_compile_options(kirbyrecomp_kfl PRIVATE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
    -Wno-character-conversion
)

# Install headers
install(FILES
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/kfl/kfl_nso.h
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/kfl/kfl_save.h
    DESTINATION include/kirbyrecomp/kfl
)