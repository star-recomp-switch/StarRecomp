# KirbyRecomp Runtime Library

cmake_minimum_required(VERSION 3.25)

# Target: kirbyrecomp_rt
add_library(kirbyrecomp_rt STATIC)

target_sources(kirbyrecomp_rt PRIVATE
    runtime.cpp
    memory.cpp
    threading.cpp
    svc_thread.cpp
)

target_include_directories(kirbyrecomp_rt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_rt PUBLIC
    kirbyrecomp_codegen
    sr_runtime
    sr_common
    SDL3::SDL3
    Vulkan::Vulkan
    VMA::VMA
    Opus::opus
    FFmpeg::FFmpeg
)

target_compile_features(kirbyrecomp_rt PUBLIC cxx_std_20)
target_compile_definitions(kirbyrecomp_rt PUBLIC KIRBYRECOMP_BUILD_RT)

# Install headers
install(FILES
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/runtime.h
    DESTINATION include/kirbyrecomp
)

install(TARGETS kirbyrecomp_rt
    EXPORT KirbyRecompTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)