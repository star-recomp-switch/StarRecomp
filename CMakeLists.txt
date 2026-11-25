set(SR_BUILD_BENCHMARKS OFF CACHE BOOL "Build benchmarks" FORCE)

cmake_minimum_required(VERSION 3.25)

# Allow overriding generator and make program from command line
if(NOT CMAKE_GENERATOR)
  set(CMAKE_GENERATOR "Ninja" CACHE STRING "Generator to use" FORCE)
endif()
if(NOT CMAKE_MAKE_PROGRAM AND CMAKE_GENERATOR STREQUAL "Ninja")
  # Try to find ninja in common locations
  find_program(NINJA_EXE ninja ninja.exe
    PATHS
    "$ENV{MSYS64}/usr/bin"
    "$ENV{DEVKITPRO}/msys2/usr/bin"
    "/c/msys64/usr/bin"
    "/c/devkitPro/msys2/usr/bin"
    "/usr/bin"
    NO_DEFAULT_PATH
  )
  if(NINJA_EXE)
    set(CMAKE_MAKE_PROGRAM "${NINJA_EXE}" CACHE FILEPATH "Path to ninja" FORCE)
  endif()
endif()

# Allow overriding compilers from command line or environment
if(NOT CMAKE_C_COMPILER)
  find_program(CLANG_EXE clang clang.exe
    PATHS
    "$ENV{MSYS64}/mingw64/bin"
    "$ENV{DEVKITPRO}/msys2/mingw64/bin"
    "/c/msys64/mingw64/bin"
    "/c/devkitPro/msys2/mingw64/bin"
    "/usr/bin"
    NO_DEFAULT_PATH
  )
  if(CLANG_EXE)
    set(CMAKE_C_COMPILER "${CLANG_EXE}" CACHE FILEPATH "C compiler" FORCE)
  endif()
endif()
if(NOT CMAKE_CXX_COMPILER)
  find_program(CLANGXX_EXE clang++ clang++.exe
    PATHS
    "$ENV{MSYS64}/mingw64/bin"
    "$ENV{DEVKITPRO}/msys2/mingw64/bin"
    "/c/msys64/mingw64/bin"
    "/c/devkitPro/msys2/mingw64/bin"
    "/usr/bin"
    NO_DEFAULT_PATH
  )
  if(CLANGXX_EXE)
    set(CMAKE_CXX_COMPILER "${CLANGXX_EXE}" CACHE FILEPATH "C++ compiler" FORCE)
  endif()
endif()

# Detect mingw64 paths from compiler location, with fallback to known locations
if(CMAKE_CXX_COMPILER AND EXISTS "${CMAKE_CXX_COMPILER}")
  get_filename_component(CLANG_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
  get_filename_component(MINGW64_ROOT "${CLANG_BIN_DIR}" DIRECTORY)
  if(EXISTS "${MINGW64_ROOT}/include")
    set(MINGW64_INCLUDE_DIR "${MINGW64_ROOT}/include")
    # Try to detect libstdc++ version
    file(GLOB GCC_VERSION_DIRS "${MINGW64_ROOT}/include/c++/*")
    if(GCC_VERSION_DIRS)
      list(SORT GCC_VERSION_DIRS)
      list(GET GCC_VERSION_DIRS -1 LATEST_GCC_DIR)
      get_filename_component(GCC_VERSION "${LATEST_GCC_DIR}" NAME)
      set(MINGW64_CXX_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/${GCC_VERSION}")
      set(MINGW64_CXX_ARCH_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/${GCC_VERSION}/x86_64-w64-mingw32")
    endif()
    message(STATUS "Found mingw64: ${MINGW64_ROOT}")
    message(STATUS "  GCC version: ${GCC_VERSION}")
  endif()
elseif(EXISTS "/c/msys64/mingw64/include")
  # Fallback to MSYS2 default location (common case)
  set(MINGW64_ROOT "/c/msys64/mingw64")
  set(MINGW64_INCLUDE_DIR "${MINGW64_ROOT}/include")
  file(GLOB GCC_VERSION_DIRS "${MINGW64_ROOT}/include/c++/*")
  if(GCC_VERSION_DIRS)
    list(SORT GCC_VERSION_DIRS)
    list(GET GCC_VERSION_DIRS -1 LATEST_GCC_DIR)
    get_filename_component(GCC_VERSION "${LATEST_GCC_DIR}" NAME)
    set(MINGW64_CXX_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/${GCC_VERSION}")
    set(MINGW64_CXX_ARCH_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/${GCC_VERSION}/x86_64-w64-mingw32")
  endif()
  message(STATUS "Found mingw64 (fallback): ${MINGW64_ROOT}")
  message(STATUS "  GCC version: ${GCC_VERSION}")
elseif(EXISTS "/mingw64/include")
  # Fallback to Git/MSYS2 mingw64
  set(MINGW64_ROOT "/mingw64")
  set(MINGW64_INCLUDE_DIR "${MINGW64_ROOT}/include")
  file(GLOB GCC_VERSION_DIRS "${MINGW64_ROOT}/include/c++/*")
  if(GCC_VERSION_DIRS)
    list(SORT GCC_VERSION_DIRS)
    list(GET GCC_VERSION_DIRS -1 LATEST_GCC_DIR)
    get_filename_component(GCC_VERSION "${LATEST_GCC_DIR}" NAME)
    set(MINGW64_CXX_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/${GCC_VERSION}")
    set(MINGW64_CXX_ARCH_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/${GCC_VERSION}/x86_64-w64-mingw32")
  endif()
  message(STATUS "Found mingw64 (fallback): ${MINGW64_ROOT}")
  message(STATUS "  GCC version: ${GCC_VERSION}")
endif()

# Prevent CMake from adding system include paths as -isystem (they'd come before our -I)
set(CMAKE_SYSTEM_INCLUDE_PATH "")
# Add mingw64 include paths manually for clang (without -nostdinc++ to allow auto-linking of stdlib)
if(MINGW64_CXX_INCLUDE_DIR AND MINGW64_CXX_ARCH_INCLUDE_DIR AND MINGW64_INCLUDE_DIR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${MINGW64_CXX_INCLUDE_DIR} -I${MINGW64_CXX_ARCH_INCLUDE_DIR} -I${MINGW64_INCLUDE_DIR}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${MINGW64_CXX_INCLUDE_DIR} -I${MINGW64_CXX_ARCH_INCLUDE_DIR} -I${MINGW64_INCLUDE_DIR}")
else()
  message(WARNING "mingw64 include paths not fully detected; system headers may not work correctly")
endif()

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

add_subdirectory(vendor/lz4)

find_package(VMA REQUIRED)
# opus is now built from vendor/opus
find_package(FFmpeg REQUIRED)

# nlohmann_json from MSYS2 ucrt64
set(nlohmann_json_DIR /c/msys64/ucrt64/lib/cmake/nlohmann_json CACHE PATH "nlohmann_json cmake dir")
find_package(nlohmann_json REQUIRED)

# OpenSSL from MSYS2 ucrt64
set(OPENSSL_ROOT_DIR /c/msys64/ucrt64 CACHE PATH "OpenSSL root dir")
find_package(OpenSSL REQUIRED)
find_package(LZ4)

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

# VMA (Vulkan Memory Allocator) - fetch from GitHub
include(FetchContent)
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.5.0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(vma)

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