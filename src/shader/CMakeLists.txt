# Shader Translation Module
# Maxwell bytecode → SPIR-V translator

set(SHADER_SOURCES
    shader.cpp
    maxwell_decoder.cpp
    spirv_generator.cpp
)

add_library(kirbyrecomp_shader STATIC ${SHADER_SOURCES})

target_include_directories(kirbyrecomp_shader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(kirbyrecomp_shader PUBLIC
    kirbyrecomp_rt
)

# Enable C++20
target_compile_features(kirbyrecomp_shader PUBLIC cxx_std_20)

# Shader compiler tools (optional, for generating test SPIR-V)
find_program(SPIRV_VAL spirv-val)
if(SPIRV_VAL)
    message(STATUS "Found spirv-val for SPIR-V validation")
    target_compile_definitions(kirbyrecomp_shader PRIVATE HAVE_SPIRV_VAL)
endif()

# For generating SPIR-V from GLSL for testing
find_program(GLSLANG_VALIDATOR glslangValidator)
find_program(GLSLC glslc)
if(GLSLANG_VALIDATOR OR GLSLC)
    message(STATUS "Found shader compiler for test generation")
    target_compile_definitions(kirbyrecomp_shader PRIVATE HAVE_SHADER_COMPILER)
endif()

# Install headers
install(FILES
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/shader/shader.h
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/shader/maxwell_decoder.h
    ${CMAKE_SOURCE_DIR}/include/kirbyrecomp/shader/spirv_generator.h
    DESTINATION include/kirbyrecomp/shader
)

install(TARGETS kirbyrecomp_shader
    EXPORT KirbyRecompTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)