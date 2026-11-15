# NVN Runtime Module
# Implements Maxwell FIFO command buffer parsing and Vulkan translation

cmake_minimum_required(VERSION 3.20)

project(kirbyrecomp_nvn)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# NVN runtime library
add_library(kirbyrecomp_nvn STATIC
    command_buffer.cpp
    resource_manager.cpp
    command_translator.cpp
)

# Find VMA
find_package(VMA REQUIRED)

target_include_directories(kirbyrecomp_nvn PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(kirbyrecomp_nvn PUBLIC
    kirbyrecomp_shader
    Vulkan::Vulkan
    VMA::VMA
)

# Add shader module include directory
target_include_directories(kirbyrecomp_nvn PUBLIC
    ${CMAKE_SOURCE_DIR}/src/shader
)

# Compiler options
target_compile_options(kirbyrecomp_nvn PRIVATE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter
    -Wno-character-conversion
)

# Install headers
install(FILES
    command_buffer.h
    resource_manager.h
    DESTINATION include/kirbyrecomp/nvn
)