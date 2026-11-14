#ifndef KIRBYRECOMP_TRANSLATOR_DECODER_AARCH64_DECODER_H
#define KIRBYRECOMP_TRANSLATOR_DECODER_AARCH64_DECODER_H

#include "capstone_wrapper.h"
#include "../ir.h"
#include <vector>
#include <optional>
#include <span>
#include <cstdint>
#include <string>

namespace kirbyrecomp::translator {

// Forward declaration
class AArch64Decoder {
public:
    AArch64Decoder();
    ~AArch64Decoder() = default;

    // Non-copyable, non-movable (CapstoneWrapper is non-movable)
    AArch64Decoder(const AArch64Decoder&) = delete;
    AArch64Decoder& operator=(const AArch64Decoder&) = delete;
    AArch64Decoder(AArch64Decoder&&) = delete;
    AArch64Decoder& operator=(AArch64Decoder&&) = delete;

    // Decode a single instruction from bytes at the given address
    // Returns nullopt on failure
    std::optional<ir::IrInstruction> decode_instruction(
        std::span<const uint8_t> bytes,
        uint64_t address
    );

    // Decode a basic block of instructions starting at address
    // Stops at branch, end of buffer, or max_instructions
    // Returns vector of decoded instructions
    std::vector<ir::IrInstruction> decode_basic_block(
        std::span<const uint8_t> bytes,
        uint64_t start_address,
        size_t max_instructions = 256
    );

    // Get last error message from Capstone
    std::string get_last_error() const;

private:
    CapstoneWrapper capstone_;

    // Map Capstone ARM64 instruction ID to IR opcode (Capstone 5.x uses unsigned int/arm64_insn enum)
    std::optional<ir::IrOpcode> map_opcode(unsigned int capstone_id) const;

    // Convert Capstone ARM64 register to IR register index
    std::optional<uint8_t> reg_to_index(Arm64Register reg, bool& is_64bit) const;

    // Extract register operand
    std::optional<ir::IrOperand> extract_register(const Arm64Operand& op) const;

    // Extract immediate operand
    std::optional<ir::IrOperand> extract_immediate(const Arm64Operand& op) const;

    // Extract memory operand
    std::optional<ir::IrOperand> extract_memory(const Arm64Operand& op, uint8_t access_size) const;

    // Extract label operand (for branches)
    std::optional<ir::IrOperand> extract_label(const Arm64Operand& op, uint64_t pc) const;

    // Decode integer ALU instructions (ADD, SUB, AND, ORR, EOR, LSL, LSR, ASR, MUL, UDIV, SDIV)
    std::optional<ir::IrInstruction> decode_alu(const InstructionDetail& detail) const;

    // Decode load/store instructions (LDR, STR, LDP, STP)
    std::optional<ir::IrInstruction> decode_load_store(const InstructionDetail& detail) const;

    // Decode branch instructions (B, BL, BR, BLR, CBZ, CBNZ, TBZ, TBNZ)
    std::optional<ir::IrInstruction> decode_branch(const InstructionDetail& detail) const;

    // Decode compare instructions (CMP, CMN, TST)
    std::optional<ir::IrInstruction> decode_compare(const InstructionDetail& detail) const;

    // Decode move instructions (MOV, MOVK, MOVN, MOVZ)
    std::optional<ir::IrInstruction> decode_move(const InstructionDetail& detail) const;

    // Decode system instructions (SVC, HVC, SMC, MRS, MSR)
    std::optional<ir::IrInstruction> decode_system(const InstructionDetail& detail) const;

    // Helper: determine memory access size from instruction mnemonic and operands
    uint8_t get_memory_access_size(const InstructionDetail& detail) const;

    // Helper: check if register is SP (X31/SP)
    static bool is_sp(Arm64Register reg);

    // Helper: check if register is ZR (XZR/WZR)
    static bool is_zr(Arm64Register reg);
};

} // namespace kirbyrecomp::translator

#endif // KIRBYRECOMP_TRANSLATOR_DECODER_AARCH64_DECODER_H