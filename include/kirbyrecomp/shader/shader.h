#pragma once

// NVN Shader Container Format and Maxwell Bytecode Structures
// Based on NVN API documentation and envytools/envyas Maxwell ISA

#include <cstdint>
#include <vector>
#include <string>
#include <span>
#include <optional>
#include <array>

namespace kirbyrecomp::shader {

// ============================================================================
// NVN Shader Container Format
// ============================================================================

// NVN magic: "NVN8" (0x384E564E)
constexpr uint32_t NVN_SHADER_MAGIC = 0x384E564E;

// NVN shader container header (aligned to 16 bytes)
#pragma pack(push, 1)
struct NvnShaderHeader {
    uint32_t magic;           // "NVN8"
    uint32_t version;         // Container version
    uint32_t flags;           // Container flags
    uint32_t header_size;     // Size of this header
    uint32_t code_offset;     // Offset to shader code from start of container
    uint32_t code_size;       // Size of shader code
    uint32_t data_offset;     // Offset to constant data
    uint32_t data_size;       // Size of constant data
    uint32_t reloc_offset;    // Offset to relocation table
    uint32_t reloc_count;     // Number of relocations
    uint32_t symtab_offset;   // Offset to symbol table
    uint32_t symtab_count;    // Number of symbols
    uint32_t strtab_offset;   // Offset to string table
    uint32_t strtab_size;     // Size of string table
    uint32_t target_gpu;      // Target GPU architecture (0x31 = Maxwell)
    uint32_t shader_stage;    // Shader stage (vertex, fragment, compute, etc.)
    uint64_t shader_hash;     // Hash of shader code
    uint32_t reserved[6];     // Padding to 80 bytes
};
#pragma pack(pop)

// Shader stages (matching NVN API)
enum class NvnShaderStage : uint32_t {
    Vertex      = 0,
    TessControl = 1,
    TessEval    = 2,
    Geometry    = 3,
    Fragment    = 4,
    Compute     = 5,
    Count       = 6
};

// ============================================================================
// Maxwell ISA Instruction Format
// ============================================================================

// Maxwell instructions are 8 bytes (64 bits) each
// Some instructions can be 16 bytes (dual-issue pairs)

enum class MaxwellOpcode : uint8_t {
    // Data movement
    MOV    = 0x00,
    SELP   = 0x01,
    SHF    = 0x02,
    PRMT   = 0x03,

    // Integer arithmetic
    IADD   = 0x10,
    IADD3  = 0x11,
    IMAD   = 0x12,
    IMADSP = 0x13,
    IMUL   = 0x14,
    IMULH  = 0x15,
    IDIV   = 0x16,
    IREM   = 0x17,
    ISETP  = 0x18,
    ISAD   = 0x19,

    // Bitwise
    AND    = 0x20,
    OR     = 0x21,
    XOR    = 0x22,
    NOT    = 0x23,
    BFI    = 0x24,
    BFE    = 0x25,
    SHL    = 0x26,
    SHR    = 0x27,
    SHF_R  = 0x28,

    // Floating point
    FADD   = 0x30,
    FMUL   = 0x31,
    FMAD   = 0x32,
    FFMA   = 0x33,
    FDIV   = 0x34,
    FSETP  = 0x35,
    FMIN   = 0x36,
    FMAX   = 0x37,
    FSQRT  = 0x38,
    FRCP   = 0x39,
    FRSQ   = 0x3A,
    FSIN   = 0x3B,
    FCOS   = 0x3C,
    F2F    = 0x3D,  // Float to float conversion
    F2I    = 0x3E,  // Float to int
    I2F    = 0x3F,  // Int to float
    F2I_RZ = 0x40,
    F2I_RN = 0x41,
    F2I_RU = 0x42,
    F2I_RD = 0x43,

    // Comparison and selection
    SEL    = 0x50,
    SLCT   = 0x51,
    CMP    = 0x52,

    // Memory operations
    LD     = 0x60,
    ST     = 0x61,
    LDG    = 0x62,  // Load global
    STG    = 0x63,  // Store global
    LDS    = 0x64,  // Load shared
    STS    = 0x65,  // Store shared
    LDU    = 0x66,  // Load uniform
    ATOM   = 0x67,  // Atomic operations
    RED    = 0x68,  // Reduction operations

    // Texture/Sampler
    TEX    = 0x70,
    TXD    = 0x71,
    TXL    = 0x72,
    TXQ    = 0x73,
    TXF    = 0x74,
    TXB    = 0x75,
    TEXS   = 0x76,

    // Control flow
    BRA    = 0x80,
    BRX    = 0x81,
    JMP    = 0x82,
    CALL   = 0x83,
    RET    = 0x84,
    EXIT   = 0x85,
    BRK    = 0x86,
    CONT   = 0x87,
    PBRK   = 0x88,
    PCONT  = 0x89,

    // Synchronization
    BAR    = 0x90,
    SYNC   = 0x91,
    MEMBAR = 0x92,

    // Special
    NOP    = 0xF0,
    SSY    = 0xF1,
    S2R    = 0xF2,  // Special register read
    R2S    = 0xF3,  // Special register write
    VOTE   = 0xF4,
    LEPC   = 0xF5,

    // Extended (16-bit opcode space)
    EXT    = 0xFF
};

// Register types
enum class MaxwellRegType : uint8_t {
    GPR     = 0,  // General purpose register (R0-R255)
    Pred    = 1,  // Predicate register (P0-P7)
    CCR     = 2,  // Condition code register
    SR      = 3,  // Special register
    UR      = 4,  // Uniform register
    CR      = 5,  // Constant register
    TR      = 6,  // Texture register
    SR_SAM  = 7,  // Sampler register
};

// Operand encoding
struct MaxwellOperand {
    uint32_t reg_type : 4;   // MaxwellRegType
    uint32_t reg_index : 10; // Register index
    uint32_t is_immediate : 1;
    uint32_t is_negated : 1;
    uint32_t is_absolute : 1;
    uint32_t swizzle : 4;    // For vector operands (.x, .y, .z, .w)
    uint32_t modifier : 4;   // Saturation, etc.
    uint32_t reserved : 8;

    uint32_t immediate_value; // If is_immediate

    bool is_register() const { return !is_immediate; }
    bool is_predicate() const { return reg_type == static_cast<uint32_t>(MaxwellRegType::Pred); }
};

// Maxwell instruction (8 bytes = 64 bits)
#pragma pack(push, 1)
struct MaxwellInstruction {
    uint64_t raw[1];  // 8 bytes for single instruction, 16 for dual-issue

    // Decoded fields (populated by parser)
    MaxwellOpcode opcode = MaxwellOpcode::NOP;
    uint8_t pred_reg = 0;         // Predicate register (0-7), 7 = no predicate
    bool pred_neg = false;        // Predicate negation
    bool cc_update = false;       // Update condition codes
    uint8_t num_operands = 0;
    MaxwellOperand operands[8];   // Up to 8 operands

    // Helper to get raw 64-bit value
    uint64_t get_raw() const { return raw[0]; }

    // Check if this is a dual-issue instruction (16 bytes)
    bool is_dual_issue() const { return (raw[0] >> 63) & 1; }
};
#pragma pack(pop)

// ============================================================================
// Shader Reflection Information
// ============================================================================

struct ShaderInput {
    uint32_t location;
    uint32_t binding;
    uint32_t component_count;
    uint32_t format;  // SPIR-V format equivalent
    std::string name;
};

struct ShaderOutput {
    uint32_t location;
    uint32_t component_count;
    uint32_t format;
    std::string name;
};

struct ShaderResource {
    enum class Type {
        UniformBuffer,
        StorageBuffer,
        SampledImage,
        StorageImage,
        Sampler,
        PushConstant,
        InputAttachment
    };

    Type type;
    uint32_t binding;
    uint32_t set;
    uint32_t size;           // For buffers
    uint32_t array_size;     // For arrays
    std::string name;
};

struct ShaderReflection {
    NvnShaderStage stage;
    std::vector<ShaderInput> inputs;
    std::vector<ShaderOutput> outputs;
    std::vector<ShaderResource> resources;
    std::vector<uint32_t> push_constant_ranges; // (offset, size) pairs
    uint32_t local_size_x = 0;  // For compute shaders
    uint32_t local_size_y = 0;
    uint32_t local_size_z = 0;
};

// ============================================================================
// Parsed Shader Representation
// ============================================================================

struct ParsedShader {
    NvnShaderHeader header;
    std::vector<MaxwellInstruction> instructions;
    std::vector<uint8_t> constant_data;
    std::vector<uint32_t> relocations;
    ShaderReflection reflection;

    // Original bytecode for debugging
    std::vector<uint8_t> raw_bytecode;
};

// ============================================================================
// SPIR-V Generation
// ============================================================================

// SPIR-V module representation
struct SpvModule {
    std::vector<uint32_t> words;  // SPIR-V binary words

    // Metadata
    uint32_t version = 0x00010300; // SPIR-V 1.3
    uint32_t generator = 0x00010000; // Our generator magic

    // Write to buffer
    void write_to_buffer(std::vector<uint8_t>& out) const {
        out.resize(words.size() * 4);
        for (size_t i = 0; i < words.size(); ++i) {
            uint32_t w = words[i];
            out[i * 4 + 0] = static_cast<uint8_t>(w & 0xFF);
            out[i * 4 + 1] = static_cast<uint8_t>((w >> 8) & 0xFF);
            out[i * 4 + 2] = static_cast<uint8_t>((w >> 16) & 0xFF);
            out[i * 4 + 3] = static_cast<uint8_t>((w >> 24) & 0xFF);
        }
    }
};

// Shader translation options
struct ShaderTranslationOptions {
    bool optimize = true;
    bool validate_spirv = true;
    bool generate_debug_info = false;
    NvnShaderStage target_stage = NvnShaderStage::Fragment;
};

// ============================================================================
// Main Shader Translator Interface
// ============================================================================

class ShaderTranslator {
public:
    ShaderTranslator() = default;
    ~ShaderTranslator() = default;

    // Parse NVN shader container from byte array
    std::optional<ParsedShader> parse_shader_container(
        std::span<const uint8_t> data);

    // Parse raw Maxwell bytecode (without NVN container)
    std::optional<ParsedShader> parse_maxwell_bytecode(
        std::span<const uint8_t> bytecode,
        NvnShaderStage stage);

    // Translate parsed shader to SPIR-V
    std::optional<SpvModule> translate_to_spirv(
        const ParsedShader& shader,
        const ShaderTranslationOptions& options = {});

    // Extract shader reflection info
    ShaderReflection extract_reflection(const ParsedShader& shader);

    // Get last error message
    const std::string& get_last_error() const { return last_error_; }

private:
    std::string last_error_;

    // Internal parsing helpers
    bool parse_header(const uint8_t* data, size_t size, NvnShaderHeader& header);
    bool parse_instructions(const uint8_t* data, size_t size,
                           std::vector<MaxwellInstruction>& instructions);
    bool decode_instruction(const uint64_t* raw, MaxwellInstruction& inst);
    bool analyze_reflection(const ParsedShader& shader, ShaderReflection& reflection);

    // SPIR-V generation helpers
    bool generate_spirv_header(SpvModule& module, const ParsedShader& shader);
    bool generate_spirv_types(SpvModule& module, const ParsedShader& shader);
    bool generate_spirv_function(SpvModule& module, const ParsedShader& shader);
    bool translate_instruction(SpvModule& module, const MaxwellInstruction& inst);
};

} // namespace kirbyrecomp::shader