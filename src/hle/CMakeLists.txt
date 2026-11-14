# KirbyRecomp HLE Services Library

cmake_minimum_required(VERSION 3.25)

# Target: kirbyrecomp_hle
add_library(kirbyrecomp_hle STATIC)

target_sources(kirbyrecomp_hle PRIVATE
    service.cpp
)

target_include_directories(kirbyrecomp_hle PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_hle PUBLIC
    sr_common
    Vulkan::Vulkan
)

target_compile_features(kirbyrecomp_hle PUBLIC cxx_std_20)
target_compile_definitions(kirbyrecomp_hle PUBLIC KIRBYRECOMP_BUILD_HLE)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_hle SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Install headers
install(FILES
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/hle/service.h
    DESTINATION include/kirbyrecomp/hle
)

install(TARGETS kirbyrecomp_hle
    EXPORT KirbyRecompTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)