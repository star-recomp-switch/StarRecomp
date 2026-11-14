#pragma once

// Maxwell ISA Instruction Decoder
// Based on envytools/envyas and NVIDIA Maxwell documentation

#include "kirbyrecomp/shader/shader.h"
#include <cstdint>
#include <vector>
#include <string>

namespace kirbyrecomp::shader::maxwell {

// ============================================================================
// Detailed Maxwell Instruction Encoding
// ============================================================================

// Instruction format variants
enum class InstFormat : uint8_t {
    FMT_R    = 0,  // Register-register
    FMT_I    = 1,  // Register-immediate
    FMT_C    = 2,  // Register-constant
    FMT_LDST = 3,  // Load/store
    FMT_TEX  = 4,  // Texture
    FMT_CTRL = 5,  // Control flow
    FMT_SYNC = 6,  // Synchronization
    FMT_MISC = 7,  // Miscellaneous
};

// Detailed instruction information
struct InstructionInfo {
    MaxwellOpcode opcode;
    const char* mnemonic;
    InstFormat format;
    uint8_t num_src_operands;
    uint8_t num_dst_operands;
    bool has_predicate;
    bool updates_cc;
    std::string description;
};

// Opcode to info mapping
const InstructionInfo& get_instruction_info(MaxwellOpcode opcode);

// ============================================================================
// Register File
// ============================================================================

enum class RegisterFile : uint8_t {
    GPR     = 0,  // R0-R255
    PRED    = 1,  // P0-P7
    CCR     = 2,  // Condition codes
    SR      = 3,  // Special registers
    UR      = 4,  // Uniform registers (c0-c15)
    CR      = 5,  // Constant registers (c0-c15)
    TR      = 6,  // Texture registers
    SAMPLER = 7,  // Sampler registers
    ADDR    = 8,  // Address registers
    COUNT   = 9,
};

struct Register {
    RegisterFile file;
    uint16_t index;

    std::string to_string() const;
    bool operator==(const Register& other) const;
};

// ============================================================================
// Operand Types
// ============================================================================

enum class OperandType : uint8_t {
    REGISTER,
    IMMEDIATE,
    CONSTANT,
    ADDRESS,
};

struct Operand {
    OperandType type;
    Register reg;
    int64_t immediate_value;  // For immediates
    uint32_t constant_index;  // For constant buffer references
    uint32_t swizzle;         // .x/.y/.z/.w swizzle
    bool negate = false;
    bool absolute = false;
    uint8_t component_mask = 0xF;  // .xyzw mask

    std::string to_string() const;
};

// ============================================================================
// Decoded Instruction
// ============================================================================

struct DecodedInstruction {
    uint64_t address;           // Byte address in shader
    uint64_t raw[2];            // Raw instruction words (8 or 16 bytes)
    InstructionInfo info;
    std::vector<Operand> src_operands;
    std::vector<Operand> dst_operands;
    Register predicate = {RegisterFile::PRED, 7};  // P7 = always true
    bool predicate_neg = false;
    bool cc_update = false;
    bool is_dual_issue = false;
    uint32_t control_flow_target = 0;  // For BRA/BRX/CALL

    std::string to_string() const;
};

// ============================================================================
// Decoder Class
// ============================================================================

class Decoder {
public:
    Decoder() = default;
    ~Decoder() = default;

    // Decode a single instruction from 8 bytes
    std::optional<DecodedInstruction> decode_instruction(
        uint64_t address,
        const uint8_t* bytes,
        size_t available_bytes);

    // Decode a sequence of instructions
    std::vector<DecodedInstruction> decode_instructions(
        const uint8_t* bytes,
        size_t size,
        uint64_t base_address = 0);

    // Get last error
    const std::string& get_last_error() const { return last_error_; }

private:
    std::string last_error_;

    // Format-specific decoders
    bool decode_fmt_r(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_i(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_c(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_ldst(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_tex(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_ctrl(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_sync(const uint64_t* raw, DecodedInstruction& inst);
    bool decode_fmt_misc(const uint64_t* raw, DecodedInstruction& inst);

    // Operand decoders
    std::optional<Operand> decode_register_operand(uint64_t bits, int shift, int width);
    std::optional<Operand> decode_immediate_operand(uint64_t bits, int shift, int width);
    std::optional<Operand> decode_address_operand(uint64_t bits, int shift, int width);
    std::optional<Register> decode_register(uint64_t bits, int shift, int width);

    // Helper to extract bit field
    static uint64_t extract_bits(uint64_t value, int shift, int width);
};

} // namespace kirbyrecomp::shader::maxwell