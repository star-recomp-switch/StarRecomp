// NVN Shader Container Parser and Maxwell Bytecode Translator
// Implementation

#include "kirbyrecomp/shader/shader.h"
#include "kirbyrecomp/shader/maxwell_decoder.h"
#include "kirbyrecomp/shader/spirv_generator.h"

#include <cstring>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

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

    // Use the detailed Maxwell decoder for reflection analysis
    maxwell::Decoder decoder;
    auto decoded = decoder.decode_instructions(
        shader.raw_bytecode.data() + shader.header.code_offset,
        shader.header.code_size,
        0);

    // Track register usage to identify inputs/outputs/resources
    std::unordered_set<uint32_t> input_registers;   // Registers read before write
    std::unordered_set<uint32_t> output_registers;  // Registers written
    std::unordered_map<uint32_t, uint32_t> texture_registers;   // TR -> binding
    std::unordered_map<uint32_t, uint32_t> sampler_registers;   // SR_SAM -> binding
    std::unordered_map<uint32_t, uint32_t> uniform_registers;   // UR/CR -> binding
    std::unordered_map<uint32_t, uint32_t> address_registers;   // ADDR -> buffer binding
    std::unordered_set<uint32_t> global_load_registers;         // LDG src registers
    std::unordered_set<uint32_t> global_store_registers;        // STG dst registers

    // Track written registers to find outputs
    std::unordered_set<uint32_t> written_registers;

    for (const auto& inst : decoded) {
        // Track destination registers (outputs)
        for (const auto& dst : inst.dst_operands) {
            if (dst.type == maxwell::OperandType::REGISTER) {
                written_registers.insert(dst.reg.index);
                output_registers.insert(dst.reg.index);

                // Special handling for specific register files
                if (dst.reg.file == maxwell::RegisterFile::TR) {
                    // Texture register - track binding
                    texture_registers[dst.reg.index] = static_cast<uint32_t>(texture_registers.size());
                } else if (dst.reg.file == maxwell::RegisterFile::SAMPLER) {
                    sampler_registers[dst.reg.index] = static_cast<uint32_t>(sampler_registers.size());
                }
            }
        }

        // Track source registers (inputs)
        for (const auto& src : inst.src_operands) {
            if (src.type == maxwell::OperandType::REGISTER) {
                input_registers.insert(src.reg.index);

                if (src.reg.file == maxwell::RegisterFile::UR || src.reg.file == maxwell::RegisterFile::CR) {
                    uniform_registers[src.reg.index] = static_cast<uint32_t>(uniform_registers.size());
                } else if (src.reg.file == maxwell::RegisterFile::ADDR) {
                    address_registers[src.reg.index] = static_cast<uint32_t>(address_registers.size());
                } else if (src.reg.file == maxwell::RegisterFile::TR) {
                    texture_registers[src.reg.index] = static_cast<uint32_t>(texture_registers.size());
                } else if (src.reg.file == maxwell::RegisterFile::SAMPLER) {
                    sampler_registers[src.reg.index] = static_cast<uint32_t>(sampler_registers.size());
                }
            }
        }

        // Instruction-specific analysis
        switch (inst.info.opcode) {
            case MaxwellOpcode::LDG: {
                // Global load - source is address register, destination is GPR
                if (!inst.src_operands.empty() && inst.src_operands[0].type == maxwell::OperandType::REGISTER) {
                    global_load_registers.insert(inst.src_operands[0].reg.index);
                }
                if (!inst.dst_operands.empty() && inst.dst_operands[0].type == maxwell::OperandType::REGISTER) {
                    // The destination GPR receives data from a buffer
                }
                break;
            }
            case MaxwellOpcode::STG: {
                // Global store - source0 is address, source1 is value
                if (inst.src_operands.size() >= 2) {
                    if (inst.src_operands[0].type == maxwell::OperandType::REGISTER) {
                        global_store_registers.insert(inst.src_operands[0].reg.index);
                    }
                }
                break;
            }
            case MaxwellOpcode::LDU: {
                // Uniform/constant load - track uniform register usage
                if (!inst.src_operands.empty() && inst.src_operands[0].type == maxwell::OperandType::REGISTER) {
                    uniform_registers[inst.src_operands[0].reg.index] = static_cast<uint32_t>(uniform_registers.size());
                }
                break;
            }
            case MaxwellOpcode::TEX:
            case MaxwellOpcode::TXF:
            case MaxwellOpcode::TXL:
            case MaxwellOpcode::TXD:
            case MaxwellOpcode::TXB: {
                // Texture operations - track texture/sampler registers
                // Typically: dst=GPR, src0=texture_reg(TR), src1=sampler_reg(SAM), src2=coords(GPR)
                if (inst.src_operands.size() >= 2) {
                    if (inst.src_operands[0].type == maxwell::OperandType::REGISTER) {
                        texture_registers[inst.src_operands[0].reg.index] = static_cast<uint32_t>(texture_registers.size());
                    }
                    if (inst.src_operands[1].type == maxwell::OperandType::REGISTER) {
                        sampler_registers[inst.src_operands[1].reg.index] = static_cast<uint32_t>(sampler_registers.size());
                    }
                }
                break;
            }
            case MaxwellOpcode::S2R: {
                // Special register read - could be thread ID, workgroup ID, etc.
                if (!inst.src_operands.empty() && inst.src_operands[0].type == maxwell::OperandType::REGISTER &&
                    inst.src_operands[0].reg.file == maxwell::RegisterFile::SR) {
                    // Special register - built-in variable
                    uint32_t sr_index = inst.src_operands[0].reg.index;
                    ShaderInput input;
                    input.binding = 0;
                    input.format = 0; // Will be filled based on SR
                    input.name = "sr_" + std::to_string(sr_index);

                    // Map known special registers
                    switch (sr_index) {
                        case 0:  // SR_TID_X / SR_CTAID_X
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32; // R32_UINT
                            input.name = "gl_GlobalInvocationID.x";
                            break;
                        case 1:
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_GlobalInvocationID.y";
                            break;
                        case 2:
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_GlobalInvocationID.z";
                            break;
                        case 3:  // SR_CTAID_X
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_WorkGroupID.x";
                            break;
                        case 4:
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_WorkGroupID.y";
                            break;
                        case 5:
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_WorkGroupID.z";
                            break;
                        case 6:  // SR_NTID_X
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_LocalInvocationID.x";
                            break;
                        case 7:
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_LocalInvocationID.y";
                            break;
                        case 8:
                            input.location = 0;
                            input.component_count = 1;
                            input.format = 32;
                            input.name = "gl_LocalInvocationID.z";
                            break;
                        default:
                            input.location = static_cast<uint32_t>(reflection.inputs.size());
                            input.component_count = 1;
                            input.format = 32;
                            break;
                    }
                    reflection.inputs.push_back(input);
                }
                break;
            }
            case MaxwellOpcode::BAR: {
                // Barrier - indicates workgroup synchronization
                break;
            }
            case MaxwellOpcode::EXIT: {
                // Thread exit - end of shader
                break;
            }
            default:
                break;
        }
    }

    // Determine inputs: registers read but not written (or special registers)
    for (uint32_t reg : input_registers) {
        if (written_registers.find(reg) == written_registers.end()) {
            ShaderInput input;
            input.location = static_cast<uint32_t>(reflection.inputs.size());
            input.binding = 0;
            input.component_count = 4; // Assume vec4 by default
            input.format = 32; // R32G32B32A32_SFLOAT
            input.name = "in_reg_" + std::to_string(reg);
            reflection.inputs.push_back(input);
        }
    }

    // Determine outputs: registers written (especially near end of shader)
    // For fragment shaders, the last written color register is the output
    // For vertex shaders, position and other varyings are outputs
    for (uint32_t reg : output_registers) {
        ShaderOutput output;
        output.location = static_cast<uint32_t>(reflection.outputs.size());
        output.component_count = 4;
        output.format = 32;
        output.name = "out_reg_" + std::to_string(reg);
        reflection.outputs.push_back(output);
    }

    // Build resources from tracked registers
    // Uniform buffers (UR/CR)
    if (!uniform_registers.empty()) {
        ShaderResource ubo;
        ubo.type = ShaderResource::Type::UniformBuffer;
        ubo.binding = 0;
        ubo.set = 0;
        ubo.size = static_cast<uint32_t>(uniform_registers.size()) * 16; // Estimate
        ubo.array_size = 1;
        ubo.name = "UniformBuffer";
        reflection.resources.push_back(ubo);
    }

    // Storage buffers (from LDG/STG address registers)
    if (!address_registers.empty() || !global_load_registers.empty() || !global_store_registers.empty()) {
        ShaderResource ssbo;
        ssbo.type = ShaderResource::Type::StorageBuffer;
        ssbo.binding = static_cast<uint32_t>(reflection.resources.size());
        ssbo.set = 0;
        ssbo.size = 0; // Runtime sized
        ssbo.array_size = 1;
        ssbo.name = "StorageBuffer";
        reflection.resources.push_back(ssbo);
    }

    // Sampled images (texture + sampler pairs)
    size_t tex_count = std::max(texture_registers.size(), sampler_registers.size());
    for (size_t i = 0; i < tex_count; ++i) {
        ShaderResource tex;
        tex.type = ShaderResource::Type::SampledImage;
        tex.binding = static_cast<uint32_t>(reflection.resources.size());
        tex.set = 0;
        tex.size = 0;
        tex.array_size = 1;
        tex.name = "Texture" + std::to_string(i);
        reflection.resources.push_back(tex);
    }

    // Separate samplers if more samplers than textures
    if (sampler_registers.size() > texture_registers.size()) {
        for (size_t i = texture_registers.size(); i < sampler_registers.size(); ++i) {
            ShaderResource samp;
            samp.type = ShaderResource::Type::Sampler;
            samp.binding = static_cast<uint32_t>(reflection.resources.size());
            samp.set = 0;
            samp.size = 0;
            samp.array_size = 1;
            samp.name = "Sampler" + std::to_string(i);
            reflection.resources.push_back(samp);
        }
    }

    // Push constants (if small number of uniforms)
    if (uniform_registers.size() <= 16 && uniform_registers.size() > 0) {
        ShaderResource pc;
        pc.type = ShaderResource::Type::PushConstant;
        pc.binding = 0;
        pc.set = 0;
        pc.size = static_cast<uint32_t>(uniform_registers.size()) * 16;
        pc.array_size = 1;
        pc.name = "PushConstants";
        reflection.push_constant_ranges.push_back(0);  // offset
        reflection.push_constant_ranges.push_back(pc.size); // size
        reflection.resources.push_back(pc);
    }

    // Compute shader local workgroup size
    if (reflection.stage == NvnShaderStage::Compute) {
        // Try to extract from BAR instruction or special registers
        reflection.local_size_x = 64;  // Default
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

    // Use the detailed Maxwell decoder
    maxwell::Decoder decoder;
    auto decoded = decoder.decode_instructions(
        shader.raw_bytecode.data() + shader.header.code_offset,
        shader.header.code_size,
        0);

    // Use the SPIR-V generator
    spirv::ShaderTranslationOptions spirv_options;
    spirv_options.target_stage = static_cast<NvnShaderStage>(shader.header.shader_stage);
    spirv_options.local_size_x = shader.reflection.local_size_x;
    spirv_options.local_size_y = shader.reflection.local_size_y;
    spirv_options.local_size_z = shader.reflection.local_size_z;

    auto spirv_module = spirv::translate_shader(decoded,
        static_cast<NvnShaderStage>(shader.header.shader_stage),
        spirv_options);

    if (!spirv_module) {
        last_error_ = "SPIR-V translation failed";
        return std::nullopt;
    }

    // Convert to our SpvModule format
    SpvModule module;
    module.words = std::move(spirv_module->words);
    module.version = spirv_module->version;
    module.generator = spirv_module->generator;

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