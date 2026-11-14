# KirbyRecomp Codegen Library

cmake_minimum_required(VERSION 3.25)

# Target: kirbyrecomp_codegen
add_library(kirbyrecomp_codegen STATIC)

target_sources(kirbyrecomp_codegen PRIVATE
    codegen.cpp
)

target_include_directories(kirbyrecomp_codegen PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../translator
    ${CMAKE_SOURCE_DIR}/include
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_codegen PUBLIC
    sr_common
    sr_ir
    nlohmann_json::nlohmann_json
)

target_compile_features(kirbyrecomp_codegen PUBLIC cxx_std_20)
target_compile_definitions(kirbyrecomp_codegen PUBLIC KIRBYRECOMP_BUILD_CODEGEN)

# Install headers
install(FILES
    codegen.h
    DESTINATION include/kirbyrecomp/codegen
)

install(TARGETS kirbyrecomp_codegen
    EXPORT KirbyRecompTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)