# KirbyRecomp Translator Library

cmake_minimum_required(VERSION 3.25)

# Target: kirbyrecomp_translator
add_library(kirbyrecomp_translator STATIC)

target_sources(kirbyrecomp_translator PRIVATE
    decoder/capstone_wrapper.cpp
    decoder/aarch64_decoder.cpp
)

target_include_directories(kirbyrecomp_translator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/decoder
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
    /c/msys64/ucrt64/include
)

target_link_libraries(kirbyrecomp_translator PUBLIC
    sr_common
    capstone::capstone_static
)

target_compile_features(kirbyrecomp_translator PUBLIC cxx_std_20)
target_compile_definitions(kirbyrecomp_translator PUBLIC KIRBYRECOMP_BUILD_TRANSLATOR)

# Add mingw64 C++ stdlib includes for clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW64_CXX_INCLUDE_DIR)
    target_include_directories(kirbyrecomp_translator SYSTEM PRIVATE
        ${MINGW64_CXX_INCLUDE_DIR}
        ${MINGW64_CXX_ARCH_INCLUDE_DIR}
    )
endif()

# Install headers
install(FILES
    ir.h
    decoder/aarch64_decoder.h
    decoder/capstone_wrapper.h
    DESTINATION include/kirbyrecomp/translator
)

install(TARGETS kirbyrecomp_translator
    EXPORT KirbyRecompTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)