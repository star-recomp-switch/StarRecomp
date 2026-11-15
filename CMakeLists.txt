cmake_minimum_required(VERSION 3.25)
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
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-character-conversion)
endif()

# Add mingw64 system include paths for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Find mingw64 installation
  find_program(MINGW64_CLANG NAMES clang++ PATHS /c/msys64/mingw64/bin)
  if(MINGW64_CLANG)
    get_filename_component(MINGW64_ROOT "${MINGW64_CLANG}" DIRECTORY)
    get_filename_component(MINGW64_ROOT "${MINGW64_ROOT}" DIRECTORY)
    # Store paths for use in target_include_directories with SYSTEM
    set(MINGW64_INCLUDE_DIR "${MINGW64_ROOT}/include")
    set(MINGW64_CXX_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/16.2.0")
    set(MINGW64_CXX_ARCH_INCLUDE_DIR "${MINGW64_ROOT}/include/c++/16.2.0/x86_64-w64-mingw32")
    message(STATUS "Found mingw64: ${MINGW64_ROOT}")
  endif()
endif()

# Add mingw64 C++ standard library includes to targets that need C++20
function(add_mingw64_cxx_includes target)
  if(MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(${target} SYSTEM PRIVATE
      ${MINGW64_CXX_INCLUDE_DIR}
      ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
  endif()
endfunction()

# Define version macro from project version
add_compile_definitions(KIRBYRECOMP_VERSION="${PROJECT_VERSION}")

# vcpkg toolchain detection (optional, for find_package)
if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")
endif()

# Include vendor/switchrecomp (has its own CMakeLists.txt) - sets SR_INCLUDE_DIR
add_subdirectory(vendor/switchrecomp)

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