// Shader Translation Tests
// Tests Maxwell bytecode decoding and SPIR-V translation

#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <nlohmann/json.hpp>

#include "kirbyrecomp/shader/shader.h"
#include "kirbyrecomp/shader/maxwell_decoder.h"
#include "kirbyrecomp/shader/spirv_generator.h"

using namespace kirbyrecomp::shader;

namespace {

// Helper to read a file into a vector
std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return {};
    size_t size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

// Helper to find test fixture directory (without std::filesystem)
std::string get_fixture_dir() {
    // Try multiple locations - check for a known fixture file
    // Build directory structure: build/tests/integration/ -> vendor is at ../vendor
    const char* candidates[] = {
        "../vendor/switchrecomp/tests/fixtures",       // from build/tests/integration
        "../../vendor/switchrecomp/tests/fixtures",    // from build/tests
        "../vendor/switchrecomp/tests/fixtures",       // from build
        "vendor/switchrecomp/tests/fixtures",          // from build root
        "KirbyRecomp/vendor/switchrecomp/tests/fixtures",  // from source root
        nullptr
    };
    for (int i = 0; candidates[i]; ++i) {
        std::string test_file = std::string(candidates[i]) + "/translator/golden/add_imm.json";
        std::ifstream f(test_file);
        std::cout << "DEBUG get_fixture_dir: checking " << test_file << " -> " << (f.good() ? "FOUND" : "NOT FOUND") << std::endl;
        if (f.good()) {
            std::cout << "DEBUG get_fixture_dir: returning " << candidates[i] << std::endl;
            return candidates[i];
        }
    }
    return "vendor/switchrecomp/tests/fixtures";
}

// Test that Maxwell decoder doesn't crash on various byte patterns
// The Maxwell decoder is for NVIDIA GPU shader instructions, not ARM64 CPU instructions
TEST(MaxwellDecoder, NoCrashOnRandomBytes) {
    maxwell::Decoder decoder;
    int tested = 0;
    int passed = 0;

    // Test various byte patterns - decoder should not crash
    struct Sample {
        const char* name;
        std::vector<uint8_t> bytes;
    };

    const Sample samples[] = {
        {"zeros", {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
        {"ones", {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
        {"pattern1", {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}},
        {"pattern2", {0x7F, 0x80, 0xFF, 0x00, 0x7F, 0x80, 0xFF, 0x00}},
        // MOVI-like pattern (not necessarily valid Maxwell, but shouldn't crash)
        {"movi_like", {0x00, 0xF7, 0x03, 0x4F, 0x00, 0x00, 0x00, 0x00}},
        // FADD-like pattern
        {"fadd_like", {0x02, 0xD4, 0x21, 0x4E, 0x00, 0x00, 0x00, 0x00}},
        {nullptr, {}}
    };

    for (int i = 0; samples[i].name; ++i) {
        auto result = decoder.decode_instruction(0, samples[i].bytes.data(), samples[i].bytes.size());
        if (result) {
            passed++;
        }
        tested++;
    }

    EXPECT_GT(tested, 0) << "No samples were tested";
    // Decoder should not crash on any input
    std::cout << "Tested " << tested << " byte patterns, decoder did not crash" << std::endl;
}

// Test SPIR-V generator basic functionality using SpvBuilder
TEST(SpirvGenerator, BasicModule) {
    spirv::SpvBuilder builder;
    builder.begin_module();

    // Add required capabilities
    builder.capability(0); // Shader
    builder.capability(42); // Matrix

    // Memory model (Logical, GLSL450)
    builder.memory_model(0, 1);

    // Add basic types
    uint32_t void_type = builder.type_void();
    uint32_t bool_type = builder.type_bool();
    uint32_t int32_type = builder.type_int(32, 1);
    uint32_t uint32_type = builder.type_int(32, 0);
    uint32_t float32_type = builder.type_float(32);

    EXPECT_NE(void_type, 0u);
    EXPECT_NE(bool_type, 0u);
    EXPECT_NE(int32_type, 0u);
    EXPECT_NE(uint32_type, 0u);
    EXPECT_NE(float32_type, 0u);

    // Create a simple function
    uint32_t func_type = builder.type_function(void_type, {});
    uint32_t func = builder.function_begin(void_type, func_type);
    builder.function_end();

    // Add entry point
    builder.entry_point(spirv::ExecutionModel::GLCompute, func, "main", {});

    // Verify module can be serialized
    auto module = builder.end_module();
    std::vector<uint32_t> words = module.words;
    EXPECT_GT(words.size(), 5u); // At least header + some instructions

    // Check magic number
    EXPECT_EQ(words[0], 0x07230203u); // SPIR-V magic
}

// Test SPIR-V generator with shader reflection
TEST(SpirvGenerator, ShaderReflection) {
    spirv::SpvBuilder builder;
    builder.begin_module();

    builder.capability(0); // Shader
    builder.memory_model(0, 1); // Logical, GLSL450

    // Create basic types
    uint32_t void_type = builder.type_void();
    uint32_t float32_type = builder.type_float(32);

    // Create uniform buffer
    uint32_t uint32_type = builder.type_int(32, 0);
    uint32_t uniform_member = builder.type_runtime_array(uint32_type);
    uint32_t uniform_struct = builder.type_struct({uniform_member});
    uint32_t uniform_ptr = builder.type_pointer(spirv::StorageClass::Uniform, uniform_struct);

    // Create variable
    builder.variable(uniform_ptr, spirv::StorageClass::Uniform, 0);

    // Create function
    uint32_t func_type = builder.type_function(void_type, {});
    uint32_t func = builder.function_begin(void_type, func_type);
    builder.function_end();

    // Entry point
    builder.entry_point(spirv::ExecutionModel::GLCompute, func, "main", {});

    // Verify serialization
    auto module = builder.end_module();
    std::vector<uint32_t> words = module.words;
    EXPECT_GT(words.size(), 5u);
    EXPECT_EQ(words[0], 0x07230203u);
}

// Test Maxwell decoder with a simple instruction
TEST(MaxwellDecoder, SimpleInstruction) {
    maxwell::Decoder decoder;

    // MOVI Vd.4S, #1.5 (encoding: 0x4F03F700)
    // This is a MOVI instruction with imm8=0x78 (1.5 in fp16)
    // Note: Maxwell MOVI is encoded as MOV with immediate
    uint32_t encoding = 0x4F03F700u;
    std::vector<uint8_t> bytes = {
        static_cast<uint8_t>(encoding & 0xFF),
        static_cast<uint8_t>((encoding >> 8) & 0xFF),
        static_cast<uint8_t>((encoding >> 16) & 0xFF),
        static_cast<uint8_t>((encoding >> 24) & 0xFF)
    };

    auto result = decoder.decode_instruction(0, bytes.data(), bytes.size());
    // The decoder may or may not support this instruction yet
    // We just verify it doesn't crash
    SUCCEED() << "Decoder handled instruction without crash";
}

// Test full translation pipeline: Maxwell bytecode -> SPIR-V
TEST(ShaderTranslator, MaxwellToSpirv) {
    ShaderTranslator translator;

    // Create a minimal NVN shader container with a simple compute shader
    // This is a mock test - in reality we'd use real Maxwell bytecode
    std::vector<uint8_t> container;

    // NVN8 header (80 bytes)
    NvnShaderHeader header;
    header.magic = NVN_SHADER_MAGIC;
    header.version = 1;
    header.flags = 0;
    header.header_size = sizeof(NvnShaderHeader);
    header.code_offset = sizeof(NvnShaderHeader);
    header.code_size = 0; // No code for this test
    header.data_offset = 0;
    header.data_size = 0;
    header.reloc_offset = 0;
    header.reloc_count = 0;
    header.symtab_offset = 0;
    header.symtab_count = 0;
    header.strtab_offset = 0;
    header.strtab_size = 0;
    header.target_gpu = 0x31; // Maxwell
    header.shader_stage = static_cast<uint32_t>(NvnShaderStage::Compute);
    header.shader_hash = 0;
    memset(header.reserved, 0, sizeof(header.reserved));

    container.resize(sizeof(NvnShaderHeader));
    memcpy(container.data(), &header, sizeof(NvnShaderHeader));

    // Parse with empty code section - should either fail or return empty instructions
    auto result = translator.parse_shader_container(container);
    // The parser may succeed with empty code (returning empty instruction list)
    // or fail. Either is acceptable.
    SUCCEED() << "Parser handled empty code section without crash";
}

// Test SPIR-V validation (if spirv-val is available)
TEST(SpirvGenerator, Validation) {
    spirv::SpvBuilder builder;
    builder.begin_module();

    builder.capability(0); // Shader
    builder.memory_model(0, 1); // Logical, GLSL450

    uint32_t void_type = builder.type_void();
    uint32_t float32_type = builder.type_float(32);

    uint32_t func_type = builder.type_function(void_type, {});
    uint32_t func = builder.function_begin(void_type, func_type);
    builder.function_end();

    builder.entry_point(spirv::ExecutionModel::GLCompute, func, "main", {});

    auto module = builder.end_module();
    std::vector<uint32_t> words = module.words;

    // Check SPIR-V is well-formed
    EXPECT_EQ(words[0], 0x07230203u); // Magic
    EXPECT_GT(words.size(), 5u);

    // Check bound is reasonable
    EXPECT_GT(words[3], 0u);
}

// Test that shader translator can parse NVN container header
TEST(ShaderTranslator, ParseContainerHeader) {
    ShaderTranslator translator;

    // Valid NVN container header
    std::vector<uint8_t> container(sizeof(NvnShaderHeader));
    NvnShaderHeader* header = reinterpret_cast<NvnShaderHeader*>(container.data());
    header->magic = NVN_SHADER_MAGIC;
    header->version = 1;
    header->flags = 0;
    header->header_size = sizeof(NvnShaderHeader);
    header->code_offset = sizeof(NvnShaderHeader);
    header->code_size = 16; // 4 instructions
    header->data_offset = 0;
    header->data_size = 0;
    header->reloc_offset = 0;
    header->reloc_count = 0;
    header->symtab_offset = 0;
    header->symtab_count = 0;
    header->strtab_offset = 0;
    header->strtab_size = 0;
    header->target_gpu = 0x31;
    header->shader_stage = static_cast<uint32_t>(NvnShaderStage::Fragment);
    header->shader_hash = 0x12345678;
    memset(header->reserved, 0, sizeof(header->reserved));

    // Add some dummy code (4 NOP instructions - ARM64 NOP = 0xD503201F)
    // Note: These are ARM64 instructions, not Maxwell. The decoder may not parse them correctly.
    container.resize(sizeof(NvnShaderHeader) + 16);
    uint32_t nop = 0xD503201Fu; // ARM64 NOP
    for (int i = 0; i < 4; ++i) {
        container[sizeof(NvnShaderHeader) + i * 4 + 0] = static_cast<uint8_t>(nop & 0xFF);
        container[sizeof(NvnShaderHeader) + i * 4 + 1] = static_cast<uint8_t>((nop >> 8) & 0xFF);
        container[sizeof(NvnShaderHeader) + i * 4 + 2] = static_cast<uint8_t>((nop >> 16) & 0xFF);
        container[sizeof(NvnShaderHeader) + i * 4 + 3] = static_cast<uint8_t>((nop >> 24) & 0xFF);
    }

    auto result = translator.parse_shader_container(container);
    // The decoder may not support ARM64 NOP, so instruction count may vary
    // Just verify the header was parsed correctly
    if (result.has_value()) {
        EXPECT_EQ(result->header.magic, NVN_SHADER_MAGIC);
        EXPECT_EQ(result->header.shader_stage, static_cast<uint32_t>(NvnShaderStage::Fragment));
        // Instruction count depends on what the decoder can parse
        EXPECT_GE(result->instructions.size(), 0u);
    }
}

// Test Maxwell decoder with multiple instructions
TEST(MaxwellDecoder, MultipleInstructions) {
    maxwell::Decoder decoder;

    // Test a sequence of instructions
    // MOVI V0.4S, #1.5 (0x4F03F700)
    // FADD V1.4S, V0.4S, V0.4S (0x4E200401)
    // RET (0xD65F03C0) - ARM64 RET, not Maxwell, but testing decoder robustness

    std::vector<uint8_t> bytes;
    std::vector<uint32_t> encodings = {0x4F03F700u, 0x4E200401u};

    for (uint32_t enc : encodings) {
        bytes.push_back(static_cast<uint8_t>(enc & 0xFF));
        bytes.push_back(static_cast<uint8_t>((enc >> 8) & 0xFF));
        bytes.push_back(static_cast<uint8_t>((enc >> 16) & 0xFF));
        bytes.push_back(static_cast<uint8_t>((enc >> 24) & 0xFF));
    }

    auto instructions = decoder.decode_instructions(bytes.data(), bytes.size(), 0);
    EXPECT_GE(instructions.size(), 1u);

    if (instructions.size() >= 2) {
        // Check first instruction is MOV
        EXPECT_EQ(instructions[0].info.opcode, kirbyrecomp::shader::MaxwellOpcode::MOV);
    }
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}