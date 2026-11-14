// NVN Shader Container Parser and Maxwell Bytecode Translator
// Implementation

#include "kirbyrecomp/shader/shader.h"

#include <cstring>
#include <algorithm>
#include <iostream>

namespace kirbyrecomp::shader {

// ============================================================================
// ShaderTranslator Implementation
// ============================================================================

std::optional<ParsedShader> ShaderTranslator::parse_shader_container(
    std::span<const uint8_t> data) {
    last_error_.clear();

    if (data.size() < sizeof(NvnShaderHeader)) {
        last_error_ = "Data too small for NVN shader header";
        return std::nullopt;
    }

    ParsedShader shader;

    // Parse header
    const NvnShaderHeader* header_ptr = reinterpret_cast<const NvnShaderHeader*>(data.data());
    if (header_ptr->magic != NVN_SHADER_MAGIC) {
        last_error_ = "Invalid NVN shader magic";
        return std::nullopt;
    }

    shader.header = *header_ptr;
    shader.raw_bytecode.assign(data.begin(), data.end());

    // Parse shader code section
    if (shader.header.code_offset + shader.header.code_size > data.size()) {
        last_error_ = "Shader code extends beyond container";
        return std::nullopt;
    }

    const uint8_t* code_ptr = data.data() + shader.header.code_offset;
    if (!parse_instructions(code_ptr, shader.header.code_size, shader.instructions)) {
        return std::nullopt;
    }

    // Parse constant data
    if (shader.header.data_offset && shader.header.data_size) {
        if (shader.header.data_offset + shader.header.data_size <= data.size()) {
            const uint8_t* data_ptr = data.data() + shader.header.data_offset;
            shader.constant_data.assign(data_ptr, data_ptr + shader.header.data_size);
        }
    }

    // Parse relocations (simplified)
    if (shader.header.reloc_offset && shader.header.reloc_count) {
        if (shader.header.reloc_offset + shader.header.reloc_count * 4 <= data.size()) {
            const uint32_t* reloc_ptr = reinterpret_cast<const uint32_t*>(data.data() + shader.header.reloc_offset);
            shader.relocations.assign(reloc_ptr, reloc_ptr + shader.header.reloc_count);
        }
    }

    // Extract reflection info
    shader.reflection = extract_reflection(shader);

    return shader;
}

std::optional<ParsedShader> ShaderTranslator::parse_maxwell_bytecode(
    std::span<const uint8_t> bytecode,
    NvnShaderStage stage) {
    last_error_.clear();

    ParsedShader shader;
    shader.header.magic = NVN_SHADER_MAGIC;
    shader.header.version = 1;
    shader.header.shader_stage = static_cast<uint32_t>(stage);
    shader.header.code_size = static_cast<uint32_t>(bytecode.size());
    shader.header.code_offset = sizeof(NvnShaderHeader);
    shader.raw_bytecode.assign(bytecode.begin(), bytecode.end());

    if (!parse_instructions(bytecode.data(), bytecode.size(), shader.instructions)) {
        return std::nullopt;
    }

    shader.reflection = extract_reflection(shader);
    return shader;
}

bool ShaderTranslator::parse_header(const uint8_t* data, size_t size,
                                    NvnShaderHeader& header) {
    if (size < sizeof(NvnShaderHeader)) {
        last_error_ = "Insufficient data for header";
        return false;
    }

    const NvnShaderHeader* h = reinterpret_cast<const NvnShaderHeader*>(data);
    if (h->magic != NVN_SHADER_MAGIC) {
        last_error_ = "Invalid NVN magic";
        return false;
    }

    header = *h;
    return true;
}

bool ShaderTranslator::parse_instructions(const uint8_t* data, size_t size,
                                          std::vector<MaxwellInstruction>& instructions) {
    if (size % 8 != 0) {
        last_error_ = "Shader code size not multiple of 8 bytes";
        return false;
    }

    size_t num_instructions = size / 8;
    instructions.reserve(num_instructions);

    for (size_t i = 0; i < num_instructions; ++i) {
        const uint64_t* raw = reinterpret_cast<const uint64_t*>(data + i * 8);
        MaxwellInstruction inst;
        inst.raw[0] = *raw;

        if (!decode_instruction(raw, inst)) {
            last_error_ = "Failed to decode instruction at index " + std::to_string(i);
            return false;
        }

        instructions.push_back(inst);
    }

    return true;
}

bool ShaderTranslator::decode_instruction(const uint64_t* raw, MaxwellInstruction& inst) {
    uint64_t word = raw[0];

    // Maxwell instruction format (simplified):
    // Bits [7:0]   = Opcode
    // Bits [10:8]  = Predicate register (7 = none)
    // Bit  11      = Predicate negation
    // Bit  12      = CC update
    // Bits [63:13] = Operands (variable encoding)

    uint8_t opcode_val = static_cast<uint8_t>(word & 0xFF);
    inst.opcode = static_cast<MaxwellOpcode>(opcode_val);
    inst.pred_reg = static_cast<uint8_t>((word >> 8) & 0x7);
    inst.pred_neg = (word >> 11) & 1;
    inst.cc_update = (word >> 12) & 1;

    // Decode operands (simplified - real format is more complex)
    // For now, we'll decode based on opcode
    uint64_t operand_data = word >> 13;

    // This is a simplified decoder - real Maxwell ISA has complex operand encoding
    // We'll implement basic operand extraction for common instructions
    inst.num_operands = 0;

    // Check for dual-issue (bit 63 of second word would be set)
    if (inst.is_dual_issue()) {
        // Dual-issue instruction - would need second word
        // For now, treat as single instruction
    }

    return true;
}

bool ShaderTranslator::analyze_reflection(const ParsedShader& shader,
                                          ShaderReflection& reflection) {
    reflection.stage = static_cast<NvnShaderStage>(shader.header.shader_stage);

    // TODO: Analyze instructions to extract:
    // - Input/output variables (from LD/ST instructions to specific registers)
    // - Resource bindings (from TEX, LDG, etc. instructions)
    // - Push constants (from uniform register access)
    // - Local workgroup size (for compute shaders)

    // Placeholder: set compute local size if compute shader
    if (reflection.stage == NvnShaderStage::Compute) {
        reflection.local_size_x = 64;  // Default, would be extracted from shader
        reflection.local_size_y = 1;
        reflection.local_size_z = 1;
    }

    return true;
}

ShaderReflection ShaderTranslator::extract_reflection(const ParsedShader& shader) {
    ShaderReflection reflection;
    analyze_reflection(shader, reflection);
    return reflection;
}

std::optional<SpvModule> ShaderTranslator::translate_to_spirv(
    const ParsedShader& shader,
    const ShaderTranslationOptions& options) {
    last_error_.clear();

    SpvModule module;

    if (!generate_spirv_header(module, shader)) {
        return std::nullopt;
    }

    if (!generate_spirv_types(module, shader)) {
        return std::nullopt;
    }

    if (!generate_spirv_function(module, shader)) {
        return std::nullopt;
    }

    // End of module
    module.words.push_back(0x00000005); // OpModuleEnd (5 bytes: opcode=5, word_count=1)

    if (options.validate_spirv) {
        // TODO: Validate SPIR-V with spirv-val if available
    }

    return module;
}

bool ShaderTranslator::generate_spirv_header(SpvModule& module,
                                             const ParsedShader& shader) {
    // SPIR-V header (5 words)
    // Magic number: 0x07230203
    // Version: 0x00010300 (SPIR-V 1.3)
    // Generator: 0x00010000 (our generator)
    // Bound: max ID + 1 (will be updated later)
    // Reserved: 0

    module.words.push_back(0x07230203);  // Magic
    module.words.push_back(0x00010300);  // Version
    module.words.push_back(0x00010000);  // Generator
    module.words.push_back(0x00000001);  // Bound (placeholder)
    module.words.push_back(0x00000000);  // Reserved

    return true;
}

bool ShaderTranslator::generate_spirv_types(SpvModule& module,
                                            const ParsedShader& shader) {
    // TODO: Generate type declarations based on shader reflection
    // - OpTypeVoid, OpTypeFunction
    // - OpTypeInt/OpTypeFloat for scalars
    // - OpTypeVector for vec2/3/4
    // - OpTypePointer for variables
    // - OpTypeStruct for UBOs/SSBOs
    // - OpTypeImage/OpTypeSampler for textures
    // - OpTypeRuntimeArray for array bindings

    // For now, emit minimal types for a simple shader
    uint32_t next_id = 1;

    // OpTypeVoid
    module.words.push_back((1 << 16) | 19);  // OpTypeVoid (19), word_count=2
    module.words.push_back(next_id++);       // Result ID

    // OpTypeBool
    module.words.push_back((1 << 16) | 20);  // OpTypeBool (20), word_count=2
    module.words.push_back(next_id++);       // Result ID

    // OpTypeInt 32 1 (signed 32-bit)
    module.words.push_back((1 << 16) | 21);  // OpTypeInt (21), word_count=4
    module.words.push_back(next_id++);       // Result ID
    module.words.push_back(32);              // Width
    module.words.push_back(1);               // Signed

    // OpTypeInt 32 0 (unsigned 32-bit)
    module.words.push_back((1 << 16) | 21);  // OpTypeInt (21), word_count=4
    module.words.push_back(next_id++);       // Result ID
    module.words.push_back(32);              // Width
    module.words.push_back(0);               // Signed

    // OpTypeFloat 32
    module.words.push_back((1 << 16) | 22);  // OpTypeFloat (22), word_count=3
    module.words.push_back(next_id++);       // Result ID
    module.words.push_back(32);              // Width

    // OpTypeVector for vec4 float
    module.words.push_back((1 << 16) | 23);  // OpTypeVector (23), word_count=4
    module.words.push_back(next_id++);       // Result ID
    module.words.push_back(next_id - 4);     // Component type (float)
    module.words.push_back(4);               // Component count

    // Update bound in header
    module.words[3] = next_id + 100;  // Conservative estimate

    return true;
}

bool ShaderTranslator::generate_spirv_function(SpvModule& module,
                                               const ParsedShader& shader) {
    // TODO: Generate SPIR-V function from Maxwell instructions
    // This is the core translation logic

    // For now, emit a minimal compute shader entry point
    // OpEntryPoint
    module.words.push_back((1 << 16) | 15);  // OpEntryPoint (15), word_count=4+
    module.words.push_back(0);               // Execution model (0 = Vertex, 5 = GLCompute)
    module.words.push_back(1);               // Entry point function ID (placeholder)
    module.words.push_back(0);               // Name string ID (placeholder)

    // OpExecutionMode for compute shader
    if (shader.header.shader_stage == static_cast<uint32_t>(NvnShaderStage::Compute)) {
        module.words.push_back((3 << 16) | 5);  // OpExecutionMode (5), word_count=3
        module.words.push_back(1);              // Entry point ID
        module.words.push_back(52);             // LocalSize
        // LocalSize x, y, z would follow
    }

    // Translate each Maxwell instruction to SPIR-V
    for (const auto& inst : shader.instructions) {
        if (!translate_instruction(module, inst)) {
            // Non-fatal for unknown instructions
        }
    }

    // OpFunctionEnd
    module.words.push_back((1 << 16) | 56);  // OpFunctionEnd (56), word_count=1

    return true;
}

bool ShaderTranslator::translate_instruction(SpvModule& module,
                                             const MaxwellInstruction& inst) {
    // TODO: Translate individual Maxwell instruction to SPIR-V instructions
    // This is a massive mapping task

    switch (inst.opcode) {
        case MaxwellOpcode::MOV:
            // OpCopyMemory or OpStore/OpLoad
            break;
        case MaxwellOpcode::IADD:
        case MaxwellOpcode::FADD:
            // OpIAdd / OpFAdd
            break;
        case MaxwellOpcode::IMUL:
        case MaxwellOpcode::FMUL:
            // OpIMul / OpFMul
            break;
        case MaxwellOpcode::IMAD:
        case MaxwellOpcode::FMAD:
            // OpIMad / OpFMul + OpFAdd (or OpFma if supported)
            break;
        case MaxwellOpcode::LDG:
            // OpLoad from storage buffer
            break;
        case MaxwellOpcode::STG:
            // OpStore to storage buffer
            break;
        case MaxwellOpcode::TEX:
        case MaxwellOpcode::TXF:
            // OpImageSampleImplicitLod / OpImageFetch
            break;
        case MaxwellOpcode::BRA:
        case MaxwellOpcode::BRX:
            // OpBranch / OpBranchConditional
            break;
        case MaxwellOpcode::EXIT:
        case MaxwellOpcode::RET:
            // OpReturn
            break;
        default:
            // Unknown instruction - would need to be implemented
            break;
    }

    return true;
}

} // namespace kirbyrecomp::shader