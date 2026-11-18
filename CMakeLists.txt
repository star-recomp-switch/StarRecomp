# Set the generator and make program BEFORE project() to avoid regeneration issues
set(CMAKE_GENERATOR "Ninja")
set(CMAKE_MAKE_PROGRAM "/c/devkitPro/msys2/usr/bin/ninja")

# Explicitly set the compilers BEFORE project() to avoid auto-detection issues
set(CMAKE_C_COMPILER "/c/msys64/mingw64/bin/clang.exe")
set(CMAKE_CXX_COMPILER "/c/msys64/mingw64/bin/clang++.exe")

cmake_minimum_required(VERSION 3.25)

# Pre-set mingw64 paths for clang BEFORE project()
# We hardcode the mingw64 path since we know we're using it
set(MINGW64_ROOT "/c/msys64/mingw64")
set(MINGW64_INCLUDE_DIR "${MINGW64_ROOT}/include")
set(MINGW64_CXX_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/16.2.0")
set(MINGW64_CXX_ARCH_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/16.2.0/x86_64-w64-mingw32")
message(STATUS "Found mingw64: ${MINGW64_ROOT}")
# Prevent CMake from adding system include paths as -isystem (they'd come before our -I)
set(CMAKE_SYSTEM_INCLUDE_PATH "")
# Use -nostdinc++ to disable clang's built-in standard include search
# We'll manually add the libstdc++ paths and system paths in correct order
# The order matters: libstdc++ first (for #include_next), then system includes
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -I${MINGW64_CXX_INCLUDE_DIR} -I${MINGW64_CXX_ARCH_INCLUDE_DIR} -I${MINGW64_INCLUDE_DIR}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc++ -I${MINGW64_CXX_INCLUDE_DIR} -I${MINGW64_CXX_ARCH_INCLUDE_DIR} -I${MINGW64_INCLUDE_DIR}")

project(KirbyRecomp VERSION 0.1.0 LANGUAGES CXX C)

# Enable testing at the top level so CTest discovers tests in subdirectories
enable_testing()

# Prefer Ninja generator if available
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles" OR CMAKE_GENERATOR STREQUAL "NMake Makefiles" OR CMAKE_GENERATOR STREQUAL "NMake Makefiles JOM")
  message(STATUS "Consider using Ninja generator: cmake -G Ninja ...")
endif()

# C++20 standard (required)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(KIRBYRECOMP_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# Compiler warning flags
if(MSVC)
  add_compile_options(/W4 /WX /permissive- /utf-8)
else()
  # Remove -Wpedantic as it's too aggressive for system headers
  # Add specific warning suppressions for system header issues
  add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-character-conversion -Wno-c++17-attribute-extensions -Wno-c++20-attribute-extensions -Wno-c2y-extensions -Wno-invalid-constexpr -Wno-unknown-warning-option -Wno-deprecated-builtins -Wno-suggest-override -Wno-shorten-64-to-32 -Wno-old-style-cast -Wno-unused-lambda-capture -Wno-shadow -Wno-unused-but-set-variable -Winconsistent-missing-override -Wno-switch -Wno-keyword-compat -Wno-c++14-attribute-extensions)
endif()

# Add mingw64 C++ standard library includes to targets that need C++20
function(add_mingw64_cxx_includes target)
  if(MINGW64_CXX_INCLUDE_DIR)
    # Add libstdc++ paths FIRST (BEFORE) so #include_next works for cstdlib->stdlib.h
    # The mingw64 system include (/mingw64/include) will be found automatically by clang
    target_include_directories(${target} SYSTEM BEFORE PRIVATE
      ${MINGW64_CXX_INCLUDE_DIR}
      ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
  endif()
endfunction()

# Define version macro from project version
add_compile_definitions(KIRBYRECOMP_VERSION="${PROJECT_VERSION}")

# Include vendor/switchrecomp (has its own CMakeLists.txt) - sets SR_INCLUDE_DIR
# Do this BEFORE find_package to ensure our -I options come first
add_subdirectory(vendor/switchrecomp)

# vcpkg toolchain detection (optional, for find_package)
if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")
endif()

# Add custom CMake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find required dependencies via vcpkg / system packages
find_package(Capstone REQUIRED)
find_package(SDL3 REQUIRED)

# Use wrapper for Vulkan (mingw64 provides Vulkan::Loader, we need Vulkan::Vulkan)
include(FindVulkanWrapper)

find_package(VMA REQUIRED)
find_package(opus REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(LZ4 REQUIRED)

# Add Windows system libraries for missing symbols
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # SHGetValueA -> shell32, _dupenv_s -> ucrtbase
  link_libraries(shell32 ucrtbase)
endif()

# ASTC decoder support - TODO: Use astcenc library when network available
# For now, use stub implementation (decode at runtime via runtime transcoding)
# include(FetchContent)
# FetchContent_Declare(
#     astcenc
#     URL https://github.com/ARM-software/astcenc/archive/refs/tags/v4.1.0.tar.gz
#     DOWNLOAD_EXTRACT_TIMESTAMP TRUE
# )
# set(ASTCENC_SHARED_LIB OFF CACHE BOOL "" FORCE)
# set(ASTCENC_STATIC_LIB ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(astcenc)

# Include codegen library
add_subdirectory(src/codegen)

# Include runtime library
add_subdirectory(src/rt)

# Include NSO parser
add_subdirectory(src/nso)

# Include HLE services
add_subdirectory(src/hle)

# Include shader translation module (Milestone 5)
add_subdirectory(src/shader)

# Include NVN runtime module (Milestone 6)
add_subdirectory(src/nvn)

# Include KFL (Kirby and the Forgotten Land) module loader (Milestone 8)
add_subdirectory(src/kfl)

# Generated sources directory
file(MAKE_DIRECTORY ${KIRBYRECOMP_GENERATED_DIR})

# Include all tests (includes integration subdirectory)
add_subdirectory(tests)

# Translator library for AArch64 -> IR decoding
add_subdirectory(src/translator)

# Generator tool for NSO -> C translation
add_subdirectory(src/gen)
# Note: Golden tests will be enabled once mingw64 C++ stdlib include paths are fixed
# add_subdirectory(tests/golden)

# Note: kirbyrecomp_game target will be added when src/game is implemented

# ---------------------------------------------------------------------------
# Installation (optional)
# ---------------------------------------------------------------------------
install(TARGETS kirbyrecomp_rt kirbyrecomp_codegen
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")