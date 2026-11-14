# KirbyRecomp Generator Tool

cmake_minimum_required(VERSION 3.25)

# Target: kirbyrecomp_gen
add_executable(kirbyrecomp_gen)

target_sources(kirbyrecomp_gen PRIVATE
    main.cpp
    generator.cpp
    parser.cpp
)

target_include_directories(kirbyrecomp_gen PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KIRBYRECOMP_GENERATED_DIR}
    ${SR_INCLUDE_DIR}
)

target_link_libraries(kirbyrecomp_gen PRIVATE
    sr_common
    sr_platform
    sr_ir
    sr_translator
    Capstone::Capstone
)

target_compile_features(kirbyrecomp_gen PRIVATE cxx_std_20)
target_compile_definitions(kirbyrecomp_gen PRIVATE KIRBYRECOMP_BUILD_GEN)

install(TARGETS kirbyrecomp_gen
    EXPORT KirbyRecompTargets
    RUNTIME DESTINATION bin
)